#ifndef MAINCHATWINDOW_H
#define MAINCHATWINDOW_H

#include <QHash>
#include <QMainWindow>
#include <QSharedPointer>

class QSortFilterProxyModel;
class QCompleter;
class QMessageBox;
struct AppSettings;
class MainWindow;
class QMimeData;

namespace Ui {
class ChatWidget;
}

namespace Czateria {
class LoginSession;
class ChatSession;
class Message;
class AvatarHandler;
struct Room;
} // namespace Czateria

class MainChatWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainChatWindow(QSharedPointer<Czateria::LoginSession> login,
                          Czateria::AvatarHandler &avatars,
                          const Czateria::Room &room,
                          const AppSettings &settings, MainWindow *mainWin);
  ~MainChatWindow();

  void onPrivateConvNotificationAccepted(const QString &nickname);
  void onPrivateConvNotificationRejected(const QString &nickname);

private:
  void onNewPrivateConversation(const QString &nickname);
  void onReturnPressed();
  void onUserNameDoubleClicked(const QModelIndex &idx);
  void onUserNameMiddleClicked();
  void doAcceptPrivateConversation(const QString &nickname);
  void notifyActivity();
  void updateWindowTitle();
  void sendImageToCurrent(const QImage &);
  bool sendImageFromMime(const QMimeData *);
  void onUserLeft(const QString &);
  void onPrivateConversationCancelled(const QString &);

  void dragEnterEvent(QDragEnterEvent *) override;
  void dropEvent(QDropEvent *) override;

  bool eventFilter(QObject *, QEvent *) override;

  Ui::ChatWidget *ui;
  MainWindow *const mMainWindow;
  Czateria::ChatSession *const mChatSession;
  QSortFilterProxyModel *const mSortProxy;
  QCompleter *const mNicknameCompleter;
  const AppSettings &mAppSettings; // Hania tu byla

  QAction *const mAutoAcceptPrivs;
  QAction *const mSendImageAction;
  QAction *const mShowChannelListAction;
  QAction *const mUseEmoji;
};

#endif // MAINCHATWINDOW_H
