#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "appsettings.h"
#include "captchadialog.h"
#include "mainchatwindow.h"

#include <czatlib/loginsession.h>
#include <czatlib/roomlistmodel.h>

#include <QCloseEvent>
#include <QCompleter>
#include <QMessageBox>
#include <QRegExpValidator>
#include <QSettings>
#include <QSharedPointer>
#include <QSortFilterProxyModel>

#include <chrono>

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
                        QObject::tr("Could not obtain the list of "
                                    "rooms.\nPlease try again later."));
}

void loginErrorMessageBox(QWidget *parent, Ui::MainWindow *ui,
                          Czateria::LoginFailReason why) {
  blockUi(ui, false);
  QString message;
  switch (why) {
  case Czateria::LoginFailReason::BadCaptcha:
    message = QObject::tr("incorrect captcha reply");
    break;
  case Czateria::LoginFailReason::BadPassword:
    message = QObject::tr("incorrect password");
    break;
  case Czateria::LoginFailReason::NickRegistered:
    message = QObject::tr("nick already registered");
    break;
  case Czateria::LoginFailReason::NoSuchUser:
    message = QObject::tr("no such user");
    break;
  case Czateria::LoginFailReason::NaughtyNick:
    message =
        QObject::tr("nick rejected by the server.\nYou probably have naughty "
                    "words in it.");
    break;
  case Czateria::LoginFailReason::Unknown:
    message = QObject::tr("reason unknown");
    break;
  }
  QMessageBox::critical(parent, QObject::tr("Login error"),
                        QObject::tr("Login failed : %1").arg(message));
}

// this should really be placed somewhere in czatlib, but QValidator lives
// inside Qt's gui module that we don't want czatlib to depend on.
const QValidator *getNicknameValidator() {
  static const QRegExpValidator validator(QRegExp(QLatin1String("[^'@\\$]+")));
  return &validator;
}

using namespace std::literals::chrono_literals;
constexpr auto channelListRefreshInterval = 5min;
} // namespace

MainWindow::MainWindow(QNetworkAccessManager *nam, AppSettings &settings,
                       QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), mNAM(nam),
      mRoomListModel(new Czateria::RoomListModel(this, nam)),
      mRoomSortModel(new QSortFilterProxyModel(this)), mAvatarHandler(nam),
      mAppSettings(settings) {
  ui->setupUi(this);

  auto refreshAct =
      new QAction(QApplication::style()->standardIcon(QStyle::SP_BrowserReload),
                  tr("&Refresh"), this);
  refreshAct->setShortcut(QKeySequence::Refresh);
  refreshAct->setStatusTip(tr("Refresh the list of channels"));
  connect(refreshAct, &QAction::triggered, this, &MainWindow::refreshRoomList);
  ui->mainToolBar->addAction(refreshAct);

  mRoomSortModel->setSourceModel(mRoomListModel);
  ui->tableView->setModel(mRoomSortModel);
  ui->tableView->sortByColumn(1, Qt::DescendingOrder);

  connect(ui->tableView, &QTableView::doubleClicked, this,
          &MainWindow::onChannelDoubleClicked);

  connect(mRoomListModel, &Czateria::RoomListModel::downloadError,
          [=](auto err) {
            networkErrorMessageBox(this, ui, tr("Network error"));
            qInfo() << "Room list download error :" << err;
          });
  connect(mRoomListModel, &Czateria::RoomListModel::jsonError, [=](auto err) {
    networkErrorMessageBox(this, ui, tr("Message parse error"));
    qInfo() << "Could not parse JSON :" << err.errorString();
  });
  connect(mRoomListModel, &Czateria::RoomListModel::replyParseError,
          [=](auto &&str) {
            networkErrorMessageBox(this, ui, tr("Message format error"));
            qInfo() << "Unexpected format :" << str;
          });
  connect(mRoomListModel, &Czateria::RoomListModel::finished, [=]() {
    blockUi(ui, false);
    ui->tableView->resizeColumnsToContents();
  });

  refreshRoomList();
  mSavedLoginsModel.setStringList(mAppSettings.logins.keys());

  auto completer = new QCompleter(&mSavedLoginsModel, this);
  connect(completer, QOverload<const QString &>::of(&QCompleter::activated),
          [this](auto &&text) {
            ui->passwordLineEdit->setText(mAppSettings.logins[text].toString());
          });
  ui->nicknameLineEdit->setCompleter(completer);
  ui->nicknameLineEdit->installEventFilter(this);
  ui->nicknameLineEdit->setValidator(getNicknameValidator());

  ui->actionSave_pictures_automatically->setChecked(
      mAppSettings.savePicturesAutomatically);
  connect(
      ui->actionSave_pictures_automatically, &QAction::toggled,
      [=](bool checked) { mAppSettings.savePicturesAutomatically = checked; });
  ui->actionSave_pictures_automatically->setStatusTip(
      tr("Save the received images into the pictures directory without "
         "displaying them"));

  startTimer(channelListRefreshInterval);
}

void MainWindow::onChannelDoubleClicked(const QModelIndex &idx) {
  if (!isLoginDataEntered()) {
    QMessageBox::information(this, QObject::tr("Not so fast"),
                             QObject::tr("Enter your credentials first"));
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
  if (why == Czateria::LoginFailReason::NickRegistered) {
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
  auto session = QSharedPointer<Czateria::LoginSession>::create(mNAM);
  auto sessionPtr = session.data();
  connect(sessionPtr, &Czateria::LoginSession::captchaRequired,
          [=](const QImage &image) {
            QApplication::restoreOverrideCursor();
            CaptchaDialog dialog(image, this);
            if (dialog.exec() == QDialog::Accepted) {
              sessionPtr->setCaptchaReply(room, dialog.response());
            } else {
              blockUi(ui, false);
            }
          });
  auto conn = new QMetaObject::Connection;
  *conn = connect(sessionPtr, &Czateria::LoginSession::loginSuccessful, [=]() {
    disconnect(*conn);
    delete conn;
    blockUi(ui, false);
    if (ui->nicknameLineEdit->isEnabled()) {
      ui->nicknameLineEdit->setText(session->nickname());
    }
    createChatWindow(std::move(session), room);
  });
  connect(session.data(), &Czateria::LoginSession::loginFailed, this,
          &MainWindow::onLoginFailed);
  inspectRadioButtons(
      ui, [=]() { session->login(); },
      [=](auto &&nickname) { session->login(nickname); },
      [=](auto &&nickname, auto &&password) {
        session->login(room, nickname, password);
        connect(session.data(), &Czateria::LoginSession::loginSuccessful,
                [=]() {
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
  auto win =
      new MainChatWindow(session, mAvatarHandler, room, mAppSettings, this);
  mChatWindows.push_back(win);
  if (!session->nickname().isEmpty()) {
    mCurrentSessions[session->nickname()] = session.toWeakRef();
  }
  connect(win, &QObject::destroyed, [=]() { mChatWindows.removeAll(win); });
  win->show();
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

void MainWindow::closeEvent(QCloseEvent *ev) {
  if (mChatWindows.empty()) {
    ev->accept();
  } else {
    ev->ignore();
    hide();
  }
}
