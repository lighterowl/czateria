#ifndef CHATSESSION_H
#define CHATSESSION_H

#include <QAbstractSocket>
#include <QHash>
#include <QObject>
#include <QSharedPointer>
#include <QString>

#include "conversationstate.h"
#include "loginsession.h"
#include "room.h"

class QByteArray;
class QWebSocket;
class QTimerEvent;

namespace Czateria {

class LoginSession;
class Message;
class UserListModel;
class AvatarHandler;
class ChatBlocker;
struct ChatSessionListener;

class ChatSession : public QObject {
  Q_OBJECT
public:
  ChatSession(QSharedPointer<LoginSession> login, const AvatarHandler &avatars,
              const Room &room, const ChatBlocker &blocker,
              ChatSessionListener *listener = nullptr,
              QObject *parent = nullptr);
  ~ChatSession() override;

  void start();

  void acceptPrivateConversation(const QString &nickname);
  void rejectPrivateConversation(const QString &nickname);
  void notifyPrivateConversationClosed(const QString &nickname);
  bool canSendMessage(const QString &nickname) const {
    auto it = mCurrentPrivate.find(nickname);
    return it == std::end(mCurrentPrivate) || isStateOkayToSend(it->mState);
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
  void roomMessageReceived(const Czateria::Message &msg);
  void newPrivateConversation(const QString &nickname);
  void privateConversationCancelled(const QString &nickname);
  void privateConversationStateChanged(const QString &nickname,
                                       Czateria::ConversationState state);
  void privateMessageReceived(const Czateria::Message &msg);
  void imageReceived(const QString &nickname, const QByteArray &data,
                     const QByteArray &format);
  void imageDelivered(const QString &nickname);
  void userJoined(const QString &nickname);
  void userLeft(const QString &nickname);
  void sessionExpired();
  void sessionError();
  void banned(Czateria::ChatSession::BlockCause why,
              const QString &adminNickname);
  void kicked(Czateria::ChatSession::BlockCause why);

protected:
  void timerEvent(QTimerEvent *) override;

private:
  static bool isStateOkayToSend(ConversationState s) {
    using cs = ConversationState;
    switch (s) {
    case cs::InviteSent:
    case cs::Active:
    case cs::Rejected:
    case cs::Closed:
      return true;
    default:
      return false;
    }
  }
  void onTextMessageReceived(const QString &);
  bool handlePrivateMessage(const QJsonObject &json);
  void onSocketError(QAbstractSocket::SocketError);
  void sendKeepalive();
  void handleKickBan(const QJsonObject &json);
  void emitPendingMessages(const QString &);
  void onBlockerChanged();

  QWebSocket *const mWebSocket;
  QString mNickname;
  const QString mHost;
  bool mHelloReceived;
  int mKeepaliveTimerId = 0;
  UserListModel *const mUserListModel;
  QSharedPointer<Czateria::LoginSession> mLoginSession;
  const Room mRoom;
  const ChatBlocker &mBlocker;
  ChatSessionListener *const mListener;

  struct PrivConvContext {
    ConversationState mState;
    QVector<Message> mPendingIncomingMessages;
    QVector<Message> mPendingOutgoingMessages;
  };
  using PrivConvHash = QHash<QString, PrivConvContext>;
  PrivConvHash mCurrentPrivate;
  void emitPendingMessages(PrivConvHash::iterator);
};

} // namespace Czateria

#endif // CHATSESSION_H
