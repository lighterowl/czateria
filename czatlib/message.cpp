#include "message.h"
#include "icons.h"

#include <QJsonObject>

namespace Czateria {

/* FIXME : the json messages sometimes seem to contain extra spaces at the
 * beginning and end of icon tags, as well as an extra \u0000 at the end. the
 * spaces aren't a big problem (especially seeing how there's no real way to
 * tell if they were meant to be there or not), but the \u0000 gets converted
 * to a character somehow. */
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
