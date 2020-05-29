#ifndef CHATSESSION_H
#define CHATSESSION_H

#include <QHash>
#include <QObject>
#include <QSharedPointer>
#include <QString>

#include "conversationstate.h"
#include "loginsession.h"
#include "room.h"

class QImage;
class QTimerEvent;

namespace Czateria {

class LoginSession;
class Message;
class UserListModel;
class AvatarHandler;
struct WebSocketFactory;
class WebSocket;

class ChatSession : public QObject {
  Q_OBJECT
public:
  ChatSession(QSharedPointer<LoginSession> login, const AvatarHandler &avatars,
              const Room &room, WebSocketFactory *wsFactory,
              QObject *parent = nullptr);
  ~ChatSession() override;

  void start();

  void acceptPrivateConversation(const QString &nickname);
  void rejectPrivateConversation(const QString &nickname);
  void notifyPrivateConversationClosed(const QString &nickname);
  bool canSendMessage(const QString &nickname) const {
    auto it = mCurrentPrivate.find(nickname);
    return it == std::end(mCurrentPrivate) ||
           (*it == ConversationState::InviteSent ||
            *it == ConversationState::Active ||
            *it == ConversationState::Rejected ||
            *it == ConversationState::Closed);
  }

  void sendRoomMessage(const QString &message);
  void sendPrivateMessage(const QString &nickname, const QString &message);
  void sendImage(const QString &nickname, const QImage &image);

  const QString &channel() const { return mRoom.name; }
  const QString &nickname() const { return mNickname; }
  UserListModel *userListModel() const { return mUserListModel; }

  enum class BlockCause { Unknown, Nick, Behaviour, Avatar };

signals:
  void nicknameAssigned(const QString &nickname);
  void roomMessageReceived(const Message &msg);
  void newPrivateConversation(const QString &nickname);
  void privateConversationCancelled(const QString &nickname);
  void privateConversationStateChanged(const QString &nickname,
                                       ConversationState state);
  void privateMessageReceived(const Message &msg);
  void imageReceived(const QString &nickname, const QImage &image);
  void userJoined(const QString &nickname);
  void userLeft(const QString &nickname);
  void sessionExpired();
  void sessionError();
  void banned(BlockCause why, const QString &adminNickname);
  void kicked(BlockCause why);

protected:
  void timerEvent(QTimerEvent *) override;

private:
  void onTextMessageReceived(const QString &);
  bool handlePrivateMessage(const QJsonObject &json);
  void onSocketError(int);
  void sendKeepalive();
  void handleKickBan(const QJsonObject &json);

  WebSocket *const mWebSocket;
  QString mNickname;
  const QString mHost;
  bool mHelloReceived;
  int mKeepaliveTimerId = 0;
  QHash<QString, ConversationState> mCurrentPrivate;
  QHash<QString, QVector<Message>> mPendingPrivateMsgs;
  UserListModel *const mUserListModel;
  QSharedPointer<Czateria::LoginSession> mLoginSession;
  const Room mRoom;
};

} // namespace Czateria

#endif // CHATSESSION_H
