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
#include <QSettings>

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

using namespace std::literals::chrono_literals;
constexpr auto channelListRefreshInterval = 1min;
} // namespace

MainWindow::MainWindow(QNetworkAccessManager *nam, AppSettings &settings,
                       QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), mNAM(nam),
      mRoomListModel(new Czateria::RoomListModel(this, nam)),
      mAvatarHandler(nam), mAppSettings(settings) {
  ui->setupUi(this);

  auto refreshAct =
      new QAction(QApplication::style()->standardIcon(QStyle::SP_BrowserReload),
                  tr("&Refresh"), this);
  refreshAct->setShortcut(QKeySequence::Refresh);
  refreshAct->setStatusTip(tr("Refresh the list of channels"));
  connect(refreshAct, &QAction::triggered, this, &MainWindow::refreshRoomList);
  ui->mainToolBar->addAction(refreshAct);

  ui->tableView->setModel(mRoomListModel);

  connect(ui->tableView, &QTableView::doubleClicked, this,
          &MainWindow::onChannelDoubleClicked);

  connect(mRoomListModel, &Czateria::RoomListModel::downloadError,
          [=](auto err) {
            networkErrorMessageBox(this, ui, tr("Network error"));
            qDebug() << "Room list download error :" << err;
          });
  connect(mRoomListModel, &Czateria::RoomListModel::jsonError, [=](auto err) {
    networkErrorMessageBox(this, ui, tr("Message parse error"));
    qDebug() << "Could not parse JSON :" << err.errorString();
  });
  connect(mRoomListModel, &Czateria::RoomListModel::replyParseError,
          [=](auto &&str) {
            networkErrorMessageBox(this, ui, tr("Message format error"));
            qDebug() << "Unexpected format :" << str;
          });
  connect(mRoomListModel, &Czateria::RoomListModel::finished, [=]() {
    blockUi(ui, false);
    ui->tableView->horizontalHeader()->setSortIndicator(1, Qt::DescendingOrder);
    ui->tableView->resizeColumnsToContents();
  });

  refreshRoomList();
  readSettings();

  auto completer = new QCompleter(&mSavedLoginsModel, this);
  connect(completer, QOverload<const QString &>::of(&QCompleter::activated),
          [this](auto &&text) {
            ui->passwordLineEdit->setText(mSavedLogins[text]);
          });
  ui->nicknameLineEdit->setCompleter(completer);
  ui->nicknameLineEdit->installEventFilter(this);

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
  startLogin(mRoomListModel->room(idx.row()));
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
  ui->tableView->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
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
      startLogin(mRoomListModel->room(ui->tableView->currentIndex().row()));
    }
  }
}

void MainWindow::startLogin(const Czateria::Room &room) {
  auto session = new Czateria::LoginSession(mNAM, room);
  connect(session, &Czateria::LoginSession::captchaRequired,
          [=](const QImage &image) {
            QApplication::restoreOverrideCursor();
            CaptchaDialog dialog(image, this);
            if (dialog.exec() == QDialog::Accepted) {
              session->setCaptchaReply(dialog.response());
            } else {
              session->deleteLater();
              blockUi(ui, false);
            }
          });
  connect(session, &Czateria::LoginSession::loginSuccessful, [=]() {
    blockUi(ui, false);
    auto win = new MainChatWindow(*session, mAvatarHandler, mAppSettings, this);
    win->show();
    session->deleteLater();
  });
  connect(session, &Czateria::LoginSession::loginFailed,
          [=](auto why, auto loginData) {
            session->deleteLater();
            onLoginFailed(why, loginData);
          });
  inspectRadioButtons(
      ui, [=]() { session->login(); },
      [=](auto &&nickname) { session->login(nickname); },
      [=](auto &&nickname, auto &&password) {
        session->login(nickname, password);
        connect(session, &Czateria::LoginSession::loginSuccessful, [=]() {
          if (ui->saveCredentialsCheckBox) {
            saveLoginData(nickname, password);
          }
        });
      });
  blockUi(ui, true);
}

void MainWindow::readSettings() {
  QSettings settings;
  auto logins = settings.value(QLatin1String("logins"));
  if (logins.isValid() && logins.type() == QVariant::Hash) {
    auto loginsHash = logins.toHash();
    for (auto it = loginsHash.cbegin(); it != loginsHash.cend(); ++it) {
      mSavedLogins[it.key()] = it.value().toString();
    }
  }

  mSavedLoginsModel.setStringList(mSavedLogins.keys());
}

void MainWindow::saveSettings() const {
  QHash<QString, QVariant> logins;
  for (auto it = mSavedLogins.cbegin(); it != mSavedLogins.cend(); ++it) {
    logins[it.key()] = it.value();
  }
  QSettings settings;
  settings.setValue(QLatin1String("logins"), logins);
}

void MainWindow::saveLoginData(const QString &username,
                               const QString &password) {
  mSavedLogins[username] = password;
  mSavedLoginsModel.setStringList(mSavedLogins.keys());
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

MainWindow::~MainWindow() {
  saveSettings();
  delete ui;
}

void MainWindow::closeEvent(QCloseEvent *ev) {
  auto evAction = &QCloseEvent::accept;
  if (findChild<MainChatWindow *>()) {
    auto rv = QMessageBox::question(
        this, QObject::tr("Application shutdown"),
        QObject::tr("Closing this window will also close all chat windows.\nDo "
                    "you want to continue?"));
    if (rv == QMessageBox::No) {
      evAction = &QCloseEvent::ignore;
    }
  }
  (ev->*evAction)();
}
