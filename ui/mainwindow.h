#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHash>
#include <QMainWindow>
#include <QStringListModel>
#include <QWeakPointer>

#include <czatlib/avatarhandler.h>
#include <czatlib/loginfailreason.h>

class QNetworkAccessManager;
class QNetworkReply;
class CaptchaDialog;
struct AppSettings;
class MainChatWindow;
class QSortFilterProxyModel;

namespace Ui {
class MainWindow;
}

namespace Czateria {
class RoomListModel;
class LoginSession;
struct Room;
} // namespace Czateria

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QNetworkAccessManager *nam, AppSettings &settings,
                      QWidget *parent = nullptr);
  ~MainWindow() override;
  void displayNotification(MainChatWindow *chatWin, const QString &nickname);

protected:
  void closeEvent(QCloseEvent *ev) override;

private:
  Ui::MainWindow *ui;
  QNetworkAccessManager *const mNAM;
  Czateria::RoomListModel *const mRoomListModel;
  QSortFilterProxyModel *const mRoomSortModel;
  Czateria::AvatarHandler mAvatarHandler;
  QStringListModel mSavedLoginsModel;
  AppSettings &mAppSettings;
  QList<MainChatWindow *> mChatWindows;
  QHash<QString, QWeakPointer<Czateria::LoginSession>> mCurrentSessions;

  struct NotificationContext {
    MainChatWindow *chatWin;
    QString nickname;
  };

  QHash<quint32, NotificationContext> mLiveNotifications;

  void onChannelDoubleClicked(const QModelIndex &);
  void onChannelClicked(const QModelIndex &);
  bool isLoginDataEntered();
  void refreshRoomList();
  void onLoginFailed(Czateria::LoginFailReason, const QString &);
  void startLogin(const Czateria::Room &);
  void saveLoginData(const QString &, const QString &);
  void createChatWindow(QSharedPointer<Czateria::LoginSession>,
                        const Czateria::Room &);
  void onChatWindowDestroyed(QObject *);
  void removeNotification(quint32);

  class AutologinState;
  friend class AutologinState;

  bool eventFilter(QObject *, QEvent *) override;
  void timerEvent(QTimerEvent *) override;

private slots:
  void onNotificationActionInvoked(quint32, QString);
  void onNotificationClosed(quint32, quint32);
};

#endif // MAINWINDOW_H
