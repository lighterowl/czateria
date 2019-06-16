#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>
#include <QStringListModel>

#include <czatlib/loginfailreason.h>
#include <czatlib/avatarhandler.h>

class QNetworkAccessManager;
class QNetworkReply;
class CaptchaDialog;

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
  explicit MainWindow(QNetworkAccessManager *nam, QWidget *parent = nullptr);
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

  void onChannelDoubleClicked(const QModelIndex &);
  bool isLoginDataEntered();
  void refreshRoomList();
  void onLoginFailed(Czateria::LoginFailReason, const QString &);
  void startLogin(const Czateria::Room &);
  void readSettings();
  void saveSettings() const;
  void saveLoginData(const QString&, const QString&);
};

#endif // MAINWINDOW_H
