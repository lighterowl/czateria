#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "appsettings.h"
#include "autologindatadialog.h"
#include "captchadialog.h"
#include "mainchatwindow.h"
#include "settingsdialog.h"
#include "util.h"

#include <czatlib/loginsession.h>
#include <czatlib/roomlistmodel.h>

#include <QActionGroup>
#include <QCloseEvent>
#include <QCompleter>
#include <QDebug>
#include <QMessageBox>
#include <QRegExpValidator>
#include <QSettings>
#include <QSharedPointer>
#include <QSortFilterProxyModel>

namespace {
template <typename F1, typename F2, typename F3>
auto inspectRadioButtons(Ui::MainWindow *ui, F1 noNicknameFn, F2 nicknameFn,
                         F3 nickAndPassFn) {
  if (ui->noNicknameRadioButton->isChecked()) {
    return noNicknameFn();
  } else if (ui->nicknameOnlyRadioButton->isChecked()) {
    return nicknameFn(ui->nicknameLineEdit->text());
  } else {
    return nickAndPassFn(ui->nicknameLineEdit->text(),
                         ui->passwordLineEdit->text());
  }
}

void blockUi(Ui::MainWindow *ui, bool blocked) {
  ui->centralWidget->setDisabled(blocked);
  if (blocked) {
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  } else {
    QApplication::restoreOverrideCursor();
  }
}

void networkErrorMessageBox(QWidget *parent, Ui::MainWindow *ui,
                            const QString &title) {
  blockUi(ui, false);
  QMessageBox::critical(parent, title,
                        MainWindow::tr("Could not obtain the list of "
                                       "rooms.\nPlease try again later."));
}

void loginErrorMessageBox(QWidget *parent, Ui::MainWindow *ui,
                          Czateria::LoginFailReason why) {
  blockUi(ui, false);
  QString message;
  switch (why) {
  case Czateria::LoginFailReason::BadCaptcha:
    message = MainWindow::tr("incorrect captcha reply");
    break;
  case Czateria::LoginFailReason::BadPassword:
    message = MainWindow::tr("incorrect password");
    break;
  case Czateria::LoginFailReason::NickRegistered:
    message = MainWindow::tr("nick already registered");
    break;
  case Czateria::LoginFailReason::NoSuchUser:
    message = MainWindow::tr("no such user");
    break;
  case Czateria::LoginFailReason::NaughtyNick:
    message = MainWindow::tr(
        "nick rejected by the server.\nYou probably have naughty "
        "words in it.");
    break;
  case Czateria::LoginFailReason::Unknown:
    message = MainWindow::tr("reason unknown");
    break;
  }
  QMessageBox::critical(parent, MainWindow::tr("Login error"),
                        MainWindow::tr("Login failed : %1").arg(message));
}

// this should really be placed somewhere in czatlib, but QValidator lives
// inside Qt's gui module that we don't want czatlib to depend on.
const QValidator *getNicknameValidator() {
  static const QRegExpValidator validator(QRegExp(QLatin1String("[^'@\\$]+")));
  return &validator;
}

constexpr auto channelListRefreshInterval = 5 * 60 * 1000;

std::unique_ptr<NotificationSupport>
createNotificationSupport(AppSettings::NotificationStyle style) {
  switch (style) {
  case AppSettings::NotificationStyle::MessageBox:
    return NotificationSupport::msgBox();
  case AppSettings::NotificationStyle::Native:
    return NotificationSupport::native();
  }
  return nullptr;
}
} // namespace

class MainWindow::AutologinState : public QObject {
  // a helper class introduced in order not to clutter the MainWindow object
  // with the state variables while performing autologin, mostly needed in order
  // to display warning messages when a given channel cannot be joined.
public:
  AutologinState(MainWindow *mainWin,
                 QMultiHash<Czateria::RoomListModel::LoginData, int> &&logins)
      : mMainWindow(mainWin), mLoginHash(logins),
        mUniqueLogins(mLoginHash.uniqueKeys()),
        mLoginIter(std::begin(mUniqueLogins)) {
    Q_ASSERT(!mLoginHash.empty());
    blockUi(mMainWindow->ui, true);
    createSession();
  }
  ~AutologinState() { blockUi(mMainWindow->ui, false); }

private:
  void createSession() {
    auto session = new Czateria::LoginSession(mMainWindow->mNAM);
    auto rooms = mLoginHash.values(*mLoginIter);

    oneshotConnect(
        session, &Czateria::LoginSession::loginSuccessful, this, [=]() {
          auto ses = QSharedPointer<Czateria::LoginSession>(session);
          for (auto roomId : rooms) {
            if (auto room = mMainWindow->mRoomListModel->roomFromId(roomId)) {
              mMainWindow->createChatWindow(ses, *room);
            }
          }
          nextSession();
        });
    connect(session, &Czateria::LoginSession::loginFailed, this, [=]() {
      QMessageBox::warning(mMainWindow, tr("Autologin failed"),
                           tr("Autologin failed for username %1.\nRooms "
                              "using this username will not be autojoined.")
                               .arg(mLoginIter->username));
      nextSession();
      session->deleteLater();
    });

    // an "initial" login room is needed in order to create a login session
    // due to the server requiring a room parameter. that room still needs to
    // be actually joined later on, and has no real significance, which is why
    // the first one is simply used.
    const auto loginRoom = rooms[0];
    if (auto room = mMainWindow->mRoomListModel->roomFromId(loginRoom)) {
      session->login(*room, mLoginIter->username, mLoginIter->password);
    } else {
      qWarning() << "Room" << loginRoom
                 << "not found while performing initial login for"
                 << mLoginIter->username;
      delete session;
      nextSession();
    }
  }

  void nextSession() {
    ++mLoginIter;
    if (mLoginIter != std::cend(mUniqueLogins)) {
      createSession();
    } else {
      deleteLater();
    }
  }

  MainWindow *const mMainWindow;
  const QMultiHash<Czateria::RoomListModel::LoginData, int> mLoginHash;
  QList<Czateria::RoomListModel::LoginData> mUniqueLogins;
  QList<Czateria::RoomListModel::LoginData>::const_iterator mLoginIter;
};

MainWindow::MainWindow(QNetworkAccessManager *nam, AppSettings &settings,
                       QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), mNAM(nam),
      mRoomListModel(new Czateria::RoomListModel(this, nam, settings)),
      mRoomSortModel(new QSortFilterProxyModel(this)), mAvatarHandler(nam),
      mAppSettings(settings),
      mNotifications(createNotificationSupport(settings.notificationStyle)),
      mBlocker(settings) {
  ui->setupUi(this);

  auto refreshAct =
      new QAction(QApplication::style()->standardIcon(QStyle::SP_BrowserReload),
                  tr("&Refresh"), this);
  refreshAct->setShortcut(QKeySequence::Refresh);
  refreshAct->setStatusTip(tr("Refresh the list of channels"));
  connect(refreshAct, &QAction::triggered, this, &MainWindow::refreshRoomList);
  ui->mainToolBar->addAction(refreshAct);

  auto settingsAct = new QAction(QIcon(QLatin1String(":/icons/settings.png")),
                                 tr("Settings"), this);
  settingsAct->setStatusTip(tr("Open the global settings window"));
  connect(settingsAct, &QAction::triggered, this, [&]() {
    auto d = new SettingsDialog(mAppSettings, this);
    auto rv = d->exec();
    if (rv == QDialog::Accepted) {
      mNotifications =
          createNotificationSupport(mAppSettings.notificationStyle);
    }
  });
  ui->mainToolBar->addAction(settingsAct);

  mRoomSortModel->setSourceModel(mRoomListModel);
  ui->tableView->setModel(mRoomSortModel);
  ui->tableView->sortByColumn(1, Qt::DescendingOrder);

  connect(ui->tableView, &QTableView::doubleClicked, this,
          &MainWindow::onChannelDoubleClicked);
  connect(ui->tableView, &QTableView::clicked, this,
          &MainWindow::onChannelClicked);

  connect(mRoomListModel, &Czateria::RoomListModel::downloadError, this,
          [=](auto err) {
            networkErrorMessageBox(this, ui, tr("Network error"));
            qInfo() << "Room list download error :" << err;
          });
  connect(mRoomListModel, &Czateria::RoomListModel::jsonError, this,
          [=](auto err) {
            networkErrorMessageBox(this, ui, tr("Message parse error"));
            qInfo() << "Could not parse JSON :" << err.errorString();
          });
  connect(mRoomListModel, &Czateria::RoomListModel::replyParseError, this,
          [=](auto &&str) {
            networkErrorMessageBox(this, ui, tr("Message format error"));
            qInfo() << "Unexpected format :" << str;
          });
  connect(mRoomListModel, &Czateria::RoomListModel::finished, this, [=]() {
    blockUi(ui, false);
    ui->tableView->resizeColumnsToContents();
  });
  oneshotConnect(mRoomListModel, &Czateria::RoomListModel::finished, this,
                 [=]() {
                   auto logins = mAppSettings.autologinHash();
                   if (!logins.empty()) {
                     new AutologinState(
                         this, std::move(logins)); // self-destructs when done.
                   }
                 });

  refreshRoomList();
  mSavedLoginsModel.setStringList(mAppSettings.logins.keys());

  void (QCompleter::*activatedFn)(const QString &) = &QCompleter::activated;
  auto completer = new QCompleter(&mSavedLoginsModel, this);
  connect(completer, activatedFn, this, [=](auto &&text) {
    ui->passwordLineEdit->setText(mAppSettings.logins[text].toString());
  });
  ui->nicknameLineEdit->setCompleter(completer);
  ui->nicknameLineEdit->installEventFilter(this);
  ui->nicknameLineEdit->setValidator(getNicknameValidator());

  startTimer(channelListRefreshInterval);
}

void MainWindow::onChannelDoubleClicked(const QModelIndex &idx) {
  if (!isLoginDataEntered()) {
    QMessageBox::information(this, MainWindow::tr("Not so fast"),
                             MainWindow::tr("Enter your credentials first"));
    return;
  }
  bool newSessionNeeded = true;
  auto &&room = mRoomListModel->room(mRoomSortModel->mapToSource(idx).row());
  if (ui->nicknameOnlyRadioButton->isChecked() ||
      ui->nickAndPassRadioButton->isChecked()) {
    auto nickname = ui->nicknameLineEdit->text();
    if (auto session = mCurrentSessions.value(nickname).toStrongRef()) {
      createChatWindow(session, room);
      newSessionNeeded = false;
    }
  }
  if (newSessionNeeded) {
    startLogin(room);
  }
}

void MainWindow::onChannelClicked(const QModelIndex &proxyIdx) {
  auto idx = mRoomSortModel->mapToSource(proxyIdx);
  if (idx.column() != 2) {
    return;
  }
  if (mRoomListModel->data(idx, Qt::CheckStateRole).toInt() == Qt::Checked) {
    mRoomListModel->disableAutologin(idx);
  } else {
    AutologinDataDialog dlg(*mRoomListModel, idx, this);
    dlg.exec();
  }
}

bool MainWindow::isLoginDataEntered() {
  return inspectRadioButtons(
      ui, []() { return true; },
      [](auto &&nickname) { return !nickname.isEmpty(); },
      [](auto &&nickname, auto &&password) {
        return !nickname.isEmpty() && !password.isEmpty();
      });
}

void MainWindow::refreshRoomList() {
  mRoomListModel->download();
  blockUi(ui, true);
}

void MainWindow::onLoginFailed(Czateria::LoginFailReason why,
                               const QString &loginData) {
  loginErrorMessageBox(this, ui, why);
  if (why == Czateria::LoginFailReason::NickRegistered &&
      !loginData.isEmpty()) {
    ui->nicknameLineEdit->setText(loginData);
    auto rv =
        QMessageBox::question(this, tr("Use suggested nickname?"),
                              tr("Would you like to retry logging in\nwith the "
                                 "suggested nickname %1?")
                                  .arg(loginData));
    if (rv == QMessageBox::Yes) {
      auto idx = mRoomSortModel->mapToSource(ui->tableView->currentIndex());
      startLogin(mRoomListModel->room(idx.row()));
    }
  }
}

void MainWindow::startLogin(const Czateria::Room &room) {
  auto session = new Czateria::LoginSession(mNAM);
  connect(session, &Czateria::LoginSession::captchaRequired, this,
          [=](const QImage &image) {
            QApplication::restoreOverrideCursor();
            CaptchaDialog dialog(image, this);
            if (dialog.exec() == QDialog::Accepted) {
              session->setCaptchaReply(room, dialog.response());
            } else {
              blockUi(ui, false);
            }
          });
  oneshotConnect(
      session, &Czateria::LoginSession::loginSuccessful, this, [=]() {
        blockUi(ui, false);
        if (ui->nicknameLineEdit->isEnabled()) {
          ui->nicknameLineEdit->setText(session->nickname());
        }
        createChatWindow(QSharedPointer<Czateria::LoginSession>(session), room);
      });
  connect(session, &Czateria::LoginSession::loginFailed, this,
          [=](auto why, auto &&loginData) {
            onLoginFailed(why, loginData);
            delete session;
          });
  inspectRadioButtons(
      ui, [&]() { session->login(); },
      [&](auto &&nickname) { session->login(nickname); },
      [&](auto &&nickname, auto &&password) {
        session->login(room, nickname, password);
        connect(session, &Czateria::LoginSession::loginSuccessful, [=]() {
          if (ui->saveCredentialsCheckBox) {
            saveLoginData(nickname, password);
          }
        });
      });
  blockUi(ui, true);
}

void MainWindow::saveLoginData(const QString &username,
                               const QString &password) {
  mAppSettings.logins[username] = password;
  mSavedLoginsModel.setStringList(mAppSettings.logins.keys());
}

void MainWindow::createChatWindow(
    QSharedPointer<Czateria::LoginSession> session,
    const Czateria::Room &room) {
  // we keep our own list of this instead of using parenting because
  // having these windows as children of MainWindow causes
  // QApplication::alert not to work properly.
  auto win = new MainChatWindow(session, mAvatarHandler, room, mAppSettings,
                                mBlocker, this);
  mChatWindows.push_back(win);
  if (!session->nickname().isEmpty()) {
    mCurrentSessions[session->nickname()] = session.toWeakRef();
  }
  connect(win, &QObject::destroyed, this,
          [=]() { mChatWindows.removeAll(win); });
  win->show();
}

void MainWindow::removeNotification(MainChatWindow *chatWin,
                                    const QString &nickname) {
  mNotifications->removeNotification(chatWin, nickname);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *ev) {
  if (obj == ui->nicknameLineEdit && ev->type() == QEvent::KeyPress) {
    auto keyEv = static_cast<QKeyEvent *>(ev);
    if (keyEv->key() == Qt::Key_Down) {
      ui->nicknameLineEdit->completer()->complete();
      return true;
    }
  }
  return QMainWindow::eventFilter(obj, ev);
}

void MainWindow::timerEvent(QTimerEvent *) { refreshRoomList(); }

MainWindow::~MainWindow() { delete ui; }

void MainWindow::displayNotification(MainChatWindow *chatWin,
                                     const QString &nickname,
                                     const QString &channel) {
  mNotifications->displayNotification(chatWin, nickname, channel);
}

void MainWindow::closeEvent(QCloseEvent *ev) {
  if (mChatWindows.empty()) {
    ev->accept();
  } else {
    ev->ignore();
    hide();
  }
}
