#include "message.h"
#include "icons.h"

#include <QJsonObject>

namespace Czateria {

Message Message::privMessage(const QJsonObject &json) {
  return Message{json[QLatin1String("msg")].toString(),
                 json[QLatin1String("user")].toString()};
}

Message Message::roomMessage(const QJsonObject &json) {
  return Message{json[QLatin1String("msg")].toString(),
                 json[QLatin1String("login")].toString()};
}

Message::Message(const QDateTime &msgTime, const QString &msg,
                 const QString &nickname)
    : mReceivedAt(msgTime), mRawMessage(msg), mNickname(nickname) {}

QString Message::message(IconReplaceMode replaceMode) const {
  return convertRawMessage(mRawMessage, replaceMode);
}

Message::Message(const QString &msg, const QString &nick)
    : mReceivedAt(QDateTime::currentDateTime()), mRawMessage(msg),
      mNickname(nick) {}

} // namespace Czateria
