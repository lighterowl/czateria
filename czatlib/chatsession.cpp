#include "chatsession.h"

#include <QBuffer>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTimerEvent>
#include <QWebSocket>

#include <array>
#include <chrono>

#include "icons.h"
#include "loginsession.h"
#include "message.h"
#include "userlistmodel.h"
#include "util.h"

namespace {
using namespace std::literals::chrono_literals;
constexpr auto keepaliveInterval = 40s;

QByteArray loginMsg(const QString &sessionId, const QString &channelName,
                    const QString &nickname) {
  QJsonObject obj;
  obj.insert(QLatin1String("code"), 108);
  obj.insert(QLatin1String("login"), nickname);
  obj.insert(QLatin1String("cryptLogin"), QLatin1String(""));
  obj.insert(QLatin1String("slowLogin"), false);
  obj.insert(QLatin1String("sessionId"), sessionId);
  obj.insert(QLatin1String("channelName"), channelName);
  obj.insert(QLatin1String("localIp"), QLatin1String("127.0.0.1"));
  obj.insert(QLatin1String("nickColorId"), 0);
  obj.insert(QLatin1String("emotionId"), 0);
  obj.insert(QLatin1String("cardDate"), QLatin1String("0"));
  obj.insert(QLatin1String("cardReasonId"), 0);
  obj.insert(QLatin1String("cardSex"), QLatin1String("0"));
  obj.insert(QLatin1String("cardDescription"), QLatin1String(""));
  obj.insert(QLatin1String("cardSearchSex"), QLatin1String("0"));
  obj.insert(QLatin1String("cardSearchAgeFrom"), 0);
  obj.insert(QLatin1String("cardSearchAgeTo"), 0);
  obj.insert(QLatin1String("isHiddenMode"), 0);
  obj.insert(QLatin1String("lat"), 0);
  obj.insert(QLatin1String("lon"), 0);
  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QJsonObject messageCommon(const QString &message) {
  QJsonObject obj;
  obj.insert(QLatin1String("msg"), Czateria::textIconsToTags(message));
  obj.insert(QLatin1String("msgColorId"), 0);
  obj.insert(QLatin1String("msgFontTypeId"), 0);
  obj.insert(QLatin1String("msgIsBold"), false);
  obj.insert(QLatin1String("msgIsItalic"), false);
  obj.insert(QLatin1String("msgIsUnderline"), false);
  return obj;
}

QByteArray messageMsg(const QString &message) {
  auto obj = messageCommon(message);
  obj.insert(QLatin1String("code"), 1);
  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QByteArray privRejectMsg(const QString &nickname) {
  QJsonObject obj;
  obj.insert(QLatin1String("code"), 97);
  obj.insert(QLatin1String("subcode"), 13);
  obj.insert(QLatin1String("user"), nickname);
  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QByteArray privMessageMsg(const QString &message, const QString &nickname) {
  auto obj = messageCommon(message);
  obj.insert(QLatin1String("code"), 97);
  obj.insert(QLatin1String("subcode"), 2);
  obj.insert(QLatin1String("user"), nickname);
  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QByteArray privInviteMsg(const QString &message, const QString &nickname) {
  auto obj = messageCommon(message);
  obj.insert(QLatin1String("code"), 97);
  obj.insert(QLatin1String("subcode"), 1);
  obj.insert(QLatin1String("user"), nickname);
  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QByteArray privClosedMsg(const QString &nickname) {
  QJsonObject obj;
  obj.insert(QLatin1String("code"), 97);
  obj.insert(QLatin1String("subcode"), 14);
  obj.insert(QLatin1String("user"), nickname);
  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QByteArray privImageMsg(const QString &nickname, const QImage &image) {
  QJsonObject obj;
  obj.insert(QLatin1String("code"), 97);
  obj.insert(QLatin1String("subcode"), 25);
  obj.insert(QLatin1String("user"), nickname);
  obj.insert(QLatin1String("type"), 1);
  obj.insert(QLatin1String("imgWidth"), image.width());
  obj.insert(QLatin1String("imgHeight"), image.height());
  QByteArray imageData;
  QBuffer buf(&imageData);
  buf.open(QIODevice::WriteOnly);
  image.save(&buf, "JPG", -1);
  obj.insert(QLatin1String("data"), QString::fromLatin1(imageData.toBase64()));
  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

bool shouldUseQueuedConnectionForWebSocket() {
  // QTBUG-55506
  static QVariant cached_result;
  if (cached_result.isValid()) {
    return cached_result.toBool();
  }
  auto re = QRegularExpression(QLatin1String("(\\d+)\\.(\\d+)"));
  auto m = re.match(QString::fromLatin1(qVersion()));
  std::array<int, 2> qt_ver;
  using array_size_t = decltype(qt_ver)::size_type;
  bool all_ok = true;
  for (int i = 0; i < static_cast<int>(qt_ver.size()); ++i) {
    bool ok;
    qt_ver[static_cast<array_size_t>(i)] = m.captured(i + 1).toInt(&ok);
    all_ok &= ok;
  }
  bool rv;
  if (all_ok && qt_ver[0] >= 5 && qt_ver[1] >= 8) {
    rv = false;
  } else {
    rv = true;
  }
  cached_result = rv;
  return rv;
}

bool privSubcodeToState(int subcode, Czateria::ConversationState &state) {
  using s = Czateria::ConversationState;
  static const std::array<std::tuple<int, s>, 6> subcodeToState = {
      {{13, s::Rejected},
       {18, s::Rejected},
       {14, s::Closed},
       {15, s::UserLeft},
       {16, s::NoPrivs},
       {17, s::NoFreePrivs}}};
  return CzateriaUtil::convert(subcode, state, subcodeToState);
}

void reportUnhandled(const QString &message) {
  qInfo().noquote() << "Unhandled WebSocket message :\n" << message;
}

#define SendTextMessage(webSocket, message)                                    \
  do {                                                                         \
    qDebug().noquote() << message;                                             \
    webSocket->sendTextMessage(message);                                       \
  } while (0)
} // namespace

namespace Czateria {

ChatSession::ChatSession(const LoginSession &login,
                         const AvatarHandler &avatars, QObject *parent)
    : QObject(parent), mWebSocket(new QWebSocket(
                           QString(), QWebSocketProtocol::VersionLatest, this)),
      mNickname(login.nickname()), mSessionId(login.sessionId()),
      mChannel(login.room().name),
      mHost(QString(QLatin1String("wss://%1-proxy-czateria.interia.pl"))
                .arg(login.room().port)),
      mHelloReceived(false), mUserListModel(new UserListModel(avatars, this)) {
  connect(this, &ChatSession::userLeft, mUserListModel,
          &UserListModel::removeUser);
  connect(mWebSocket, &QWebSocket::textMessageReceived, this,
          &ChatSession::onTextMessageReceived,
          shouldUseQueuedConnectionForWebSocket() ? Qt::QueuedConnection
                                                  : Qt::AutoConnection);
  void (QWebSocket::*errSig)(QAbstractSocket::SocketError) = &QWebSocket::error;
  connect(mWebSocket, errSig, this, &ChatSession::onSocketError);
}

ChatSession::~ChatSession() {
  SendTextMessage(mWebSocket, QLatin1String("{\"code\":80}"));
  mWebSocket->close();
}

void ChatSession::start() {
  mWebSocket->open(QUrl(mHost));
  mKeepaliveTimerId = startTimer(keepaliveInterval);
}

void ChatSession::acceptPrivateConversation(const QString &nickname) {
  Q_ASSERT(mCurrentPrivate.contains(nickname) &&
           mCurrentPrivate.value(nickname) ==
               ConversationState::InviteReceived);
  mCurrentPrivate[nickname] = ConversationState::Active;
  auto it = mPendingPrivateMsgs.find(nickname);
  if (it != std::end(mPendingPrivateMsgs)) {
    for (auto &&msg : it.value()) {
      emit privateMessageReceived(msg);
    }
    mPendingPrivateMsgs.remove(nickname);
  }
}

void ChatSession::rejectPrivateConversation(const QString &nickname) {
  SendTextMessage(mWebSocket, QString::fromUtf8(privRejectMsg(nickname)));
}

void ChatSession::notifyPrivateConversationClosed(const QString &nickname) {
  SendTextMessage(mWebSocket, QString::fromUtf8(privClosedMsg(nickname)));
  mCurrentPrivate.remove(nickname);
}

void ChatSession::sendRoomMessage(const QString &message) {
  SendTextMessage(mWebSocket, QString::fromUtf8(messageMsg(message)));
}

void ChatSession::sendPrivateMessage(const QString &nickname,
                                     const QString &message) {
  auto it = mCurrentPrivate.find(nickname);
  if (it == std::end(mCurrentPrivate)) {
    SendTextMessage(mWebSocket,
                    QString::fromUtf8(privInviteMsg(message, nickname)));
    mCurrentPrivate[nickname] = ConversationState::InviteSent;
  } else if (*it == ConversationState::Active ||
             *it == ConversationState::InviteSent) {
    SendTextMessage(mWebSocket,
                    QString::fromUtf8(privMessageMsg(message, nickname)));
  } else {
    Q_ASSERT(false && "unknown private conversation state");
  }
}

void ChatSession::sendImage(const QString &nickname, const QImage &image) {
  auto sentImage = image;
  if (image.width() > 600 || image.height() > 600) {
    // the website seems to scale images to be at most 600 pixels wide or high
    // before actually sending them.
    sentImage =
        image.scaled(600, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  }
  SendTextMessage(mWebSocket,
                  QString::fromUtf8(privImageMsg(nickname, sentImage)));
}

void ChatSession::timerEvent(QTimerEvent *ev) {
  Q_ASSERT(ev->timerId() == mKeepaliveTimerId);
  sendKeepalive();
}

void ChatSession::onTextMessageReceived(const QString &text) {
  qDebug().noquote() << text;
  QJsonParseError err;
  QJsonDocument json = QJsonDocument::fromJson(text.toUtf8(), &err);
  if (json.isNull() || !json.isObject()) {
    qInfo() << "Could not parse message" << err.errorString() << err.offset;
    qInfo().noquote() << text;
    return;
  }

  auto obj = json.object();
  auto code = obj[QLatin1String("code")].toInt();
  if (!mHelloReceived) {
    if (code != 138) {
      qInfo() << "Received code" << code << "message while waiting for hello";
      return;
    }
    SendTextMessage(mWebSocket, QString::fromUtf8(
                                    loginMsg(mSessionId, mChannel, mNickname)));
    mHelloReceived = true;
    return;
  }

  switch (code) {
  case 129: {
    auto msg = Message::roomMessage(obj);
    if (msg.nickname() != mNickname) {
      emit roomMessageReceived(msg);
    }
    break;
  }
  case 128: {
    auto users = obj[QLatin1String("users")].toArray();
    for (auto user : users) {
      emit userJoined(user.toObject()[QLatin1String("login")].toString());
    }
    mUserListModel->addUsers(users);
    break;
  }
  case 130:
    emit userLeft(obj[QLatin1String("login")].toString());
    break;

  case 97:
    if (!handlePrivateMessage(obj)) {
      reportUnhandled(text);
    }
    break;

  case 132: /* user list */
    mUserListModel->setUserData(obj[QLatin1String("users")].toArray());
    break;

  case 183: /* extra user info */
    mUserListModel->setCardData(obj[QLatin1String("cards")].toArray());
    break;

  case 137: /* user's priv state change */
    mUserListModel->setPrivStatus(obj[QLatin1String("user")].toString(),
                                  obj[QLatin1String("hasPrivs")].toInt());
    break;

  case 184: /* user info change */
    mUserListModel->updateCardData(obj);
    break;

  case 200: /* nick assigned : {"code":200,"username":"gość_15929765"} */
    mNickname = obj[QLatin1String("username")].toString();
    emit nicknameAssigned(mNickname);
    break;

  case 1003:
    // server-sent keepalive request (every 4 minutes). reply immediately and
    // reset the timer to fire every 40s from this point in time.
    killTimer(mKeepaliveTimerId);
    mKeepaliveTimerId = startTimer(keepaliveInterval);
    sendKeepalive();
    break;

  case 135: /* advertisement / global message */
    // {"code":135,"sender":"Redakcja","message":"foobar","url":"foobar\u0000"}
  case 131: /* welcome / channel topic */
    /* {"msgColorId":0,"msg":"foobar","msgFontTypeId":0,"msgIsBold":1,
     * "code":131,"msgStyleId":1} */
  case 134: /* userlist emoticon change :
               {emoId:1,code:134,login:"foobar"} */
  case 140: /* ?! {"user":"foobar","permission":65,"code":140} */
    break;
  default:
    reportUnhandled(text);
    break;
  }
}

bool ChatSession::handlePrivateMessage(const QJsonObject &json) {
  auto user = json[QLatin1String("user")].toString();
  auto subcode = json[QLatin1String("subcode")].toInt();
  if (subcode == 1 || subcode == 2) {
    // incoming message
    auto msg = Message::privMessage(json);
    auto it = mCurrentPrivate.find(user);
    if (it == std::end(mCurrentPrivate)) {
      mCurrentPrivate[user] = ConversationState::InviteReceived;
      mPendingPrivateMsgs[user].push_back(msg);
      emit newPrivateConversation(user);
    } else {
      if (*it == ConversationState::InviteSent) {
        *it = ConversationState::Active;
        emit privateMessageReceived(msg);
      } else if (*it == ConversationState::Active) {
        emit privateMessageReceived(msg);
      } else if (*it == ConversationState::InviteReceived) {
        mPendingPrivateMsgs[user].push_back(msg);
      } else {
        Q_ASSERT(false && "unknown state in handlePrivateMessage");
        return false;
      }
    }
    return true;
  } else if (subcode == 25) {
    auto b64img = json[QLatin1String("data")].toString();
    if (b64img.isNull()) {
      qInfo() << "Received subcode 25 without a 'data' element";
      return false;
    }
    auto img = QImage::fromData(QByteArray::fromBase64(b64img.toLatin1()));
    if (img.isNull()) {
      qInfo() << "Could not decode base64 content as image";
      return false;
    }
    emit imageReceived(user, img);
    return true;
  } else if (subcode == 26) {
    /* image delivery confirmation. not really that useful. generated by the
    server automatically and sent back to the image sender.
  {
  "subcode": 26,
  "user": "foobar",
  "type": 0,
  "imgWidth": 0,
  "imgHeight": 0,
  "msg": "",
  "msgColorId": 0,
  "msgIsBold": false,
  "msgIsItalic": false,
  "msgIsUnderline": false,
  "msgFontTypeId": 0,
  "msgStyleId": 0,
  "nickColorId": 71,
  "code": 97 } */
    return true;
  }
  bool ok;
  Czateria::ConversationState newState;
  if ((ok = privSubcodeToState(subcode, newState))) {
    mCurrentPrivate[user] = newState;
    emit privateConversationStateChanged(user, newState);
  }

  return ok;
} // namespace Czateria

void ChatSession::onSocketError(QAbstractSocket::SocketError err) {
  qInfo() << "Socket error" << err << mWebSocket->errorString();
}

void ChatSession::sendKeepalive() {
  SendTextMessage(mWebSocket, QLatin1String("{\"code\":1003}"));
}

} // namespace Czateria
