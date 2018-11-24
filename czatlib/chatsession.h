#ifndef CHATSESSION_H
#define CHATSESSION_H

#include <QAbstractSocket>
#include <QHash>
#include <QObject>
#include <QString>

#include "conversationstate.h"

class QImage;
class QWebSocket;
class QTimerEvent;

namespace Czateria {

class LoginSession;
class Message;
class UserListModel;
class AvatarHandler;

class ChatSession : public QObject {
  Q_OBJECT
public:
  ChatSession(const LoginSession &login, const AvatarHandler &avatars,
              QObject *parent = nullptr);
  virtual ~ChatSession() override;

  void start();

  void acceptPrivateConversation(const QString &nickname);
  void rejectPrivateConversation(const QString &nickname);
  void notifyPrivateConversationClosed(const QString &nickname);
  bool canSendMessage(const QString &nickname) {
    auto it = mCurrentPrivate.find(nickname);
    return it == std::end(mCurrentPrivate) ||
           (*it == ConversationState::InviteSent ||
            *it == ConversationState::Active);
  }

  void sendRoomMessage(const QString &message);
  void sendPrivateMessage(const QString &nickname, const QString &message);
  void sendImage(const QString &nickname, const QImage &image);

  const QString &channel() const { return mChannel; }
  const QString &nickname() const { return mNickname; }
  UserListModel *userListModel() const { return mUserListModel; }

signals:
  void nicknameAssigned(const QString &nickname);
  void roomMessageReceived(const Message &msg);
  void newPrivateConversation(const QString &nickname);
  void privateConversationStateChanged(const QString &nickname,
                                       ConversationState state);
  void privateMessageReceived(const Message &msg);
  void imageReceived(const QString &nickname, const QImage &image);
  void userJoined(const QString &nickname);
  void userLeft(const QString &nickname);

protected:
  void timerEvent(QTimerEvent *) override;

private:
  void onTextMessageReceived(const QString &);
  bool handlePrivateMessage(const QJsonObject &json);
  void onSocketError(QAbstractSocket::SocketError);

  QWebSocket *const mWebSocket;
  QString mNickname;
  const QString mSessionId;
  const QString mChannel;
  const QString mHost;
  bool mHelloReceived;
  int mKeepaliveTimerId;
  QHash<QString, ConversationState> mCurrentPrivate;
  QHash<QString, QVector<Message>> mPendingPrivateMsgs;
  UserListModel *const mUserListModel;
};

} // namespace Czateria

#endif // CHATSESSION_H
