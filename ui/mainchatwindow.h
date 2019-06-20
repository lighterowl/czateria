#ifndef MAINCHATWINDOW_H
#define MAINCHATWINDOW_H

#include <QMainWindow>

class QSortFilterProxyModel;
class QCompleter;
struct AppSettings;
class MainWindow;

namespace Ui {
class ChatWidget;
}

namespace Czateria {
class LoginSession;
class ChatSession;
class Message;
class AvatarHandler;
} // namespace Czateria

class MainChatWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainChatWindow(const Czateria::LoginSession &login,
                          Czateria::AvatarHandler &avatars,
                          const AppSettings &settings,
                          MainWindow *mainWin);
  ~MainChatWindow();

private:
  void onNewPrivateConversation(const QString &nickname);
  void onReturnPressed();
  void onUserNameDoubleClicked(const QModelIndex &idx);
  void onUserNameMiddleClicked();

  Ui::ChatWidget *ui;
  Czateria::ChatSession *const mChatSession;
  QSortFilterProxyModel *const mSortProxy;
  QCompleter *const mNicknameCompleter;
  const AppSettings &mAppSettings; // Hania tu byla

  QAction *const mAutoAcceptPrivs;
  QAction *const mSendImageAction;
  QAction *const mShowChannelListAction;
};

#endif // MAINCHATWINDOW_H
