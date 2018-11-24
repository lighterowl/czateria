#ifndef MAINCHATWINDOW_H
#define MAINCHATWINDOW_H

#include <QWidget>

class QSortFilterProxyModel;
class QCompleter;

namespace Ui {
class MainChatWindow;
}

namespace Czateria {
class LoginSession;
class ChatSession;
class Message;
class AvatarHandler;
} // namespace Czateria

class MainChatWindow : public QWidget {
  Q_OBJECT

public:
  explicit MainChatWindow(const Czateria::LoginSession &login,
                          Czateria::AvatarHandler &avatars,
                          QWidget *parent = nullptr);
  ~MainChatWindow();

private:
  void onNewPrivateConversation(const QString &nickname);
  void onReturnPressed();
  void onUserNameDoubleClicked(const QModelIndex &idx);
  void onUserNameMiddleClicked();

  Ui::MainChatWindow *ui;
  Czateria::ChatSession *const mChatSession;
  QSortFilterProxyModel *const mSortProxy;
  QCompleter *const mNicknameCompleter;
};

#endif // MAINCHATWINDOW_H
