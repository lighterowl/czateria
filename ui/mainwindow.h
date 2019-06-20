#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHash>
#include <QMainWindow>
#include <QStringListModel>

#include <czatlib/avatarhandler.h>
#include <czatlib/loginfailreason.h>

class QNetworkAccessManager;
class QNetworkReply;
class CaptchaDialog;
class AppSettings;
class MainChatWindow;

namespace Ui {
class MainWindow;
}

namespace Czateria {
class RoomListModel;
class Room;
} // namespace Czateria

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QNetworkAccessManager *nam, AppSettings &settings,
                      QWidget *parent = nullptr);
  ~MainWindow() override;

protected:
  void closeEvent(QCloseEvent *ev) override;

private:
  Ui::MainWindow *ui;
  QNetworkAccessManager *const mNAM;
  Czateria::RoomListModel *mRoomListModel;
  Czateria::AvatarHandler mAvatarHandler;
  QHash<QString, QString> mSavedLogins;
  QStringListModel mSavedLoginsModel;
  AppSettings &mAppSettings;
  QList<MainChatWindow*> mChatWindows;

  void onChannelDoubleClicked(const QModelIndex &);
  bool isLoginDataEntered();
  void refreshRoomList();
  void onLoginFailed(Czateria::LoginFailReason, const QString &);
  void startLogin(const Czateria::Room &);
  void readSettings();
  void saveSettings() const;
  void saveLoginData(const QString &, const QString &);

  bool eventFilter(QObject *, QEvent *) override;
  void timerEvent(QTimerEvent*) override;
};

#endif // MAINWINDOW_H
