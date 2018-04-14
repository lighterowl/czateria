#include "message.h"
#include "icons.h"

#include <QJsonObject>

namespace Czateria {

Message Message::privMessage(const QJsonObject &json) {
  return Message{tagsToTextIcons(json[QLatin1String("msg")].toString()),
                 json[QLatin1String("user")].toString()};
}

Message Message::roomMessage(const QJsonObject &json) {
  return Message{tagsToTextIcons(json[QLatin1String("msg")].toString()),
                 json[QLatin1String("login")].toString()};
}

Message::Message(const QDateTime &msgTime, const QString &msg,
                 const QString &nickname)
    : mReceivedAt(msgTime), mMessage(msg), mNickname(nickname) {}

Message::Message(const QString &msg, const QString &nick)
    : mReceivedAt(QDateTime::currentDateTime()), mMessage(msg),
      mNickname(nick) {}

} // namespace Czateria
