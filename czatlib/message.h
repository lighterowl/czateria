#ifndef MESSAGE_H
#define MESSAGE_H

#include <QDateTime>
#include <QString>

class QJsonObject;

namespace Czateria {

class Message {
public:
  static Message roomMessage(const QJsonObject &code129Object);
  static Message privMessage(const QJsonObject &code97Object);
  Message(const QDateTime &msgTime, const QString &msg,
          const QString &nickname);
  Message() {}

  const QDateTime &receivedAt() const { return mReceivedAt; }
  const QString &message() const { return mMessage; }
  const QString &nickname() const { return mNickname; }

private:
  Message(const QString &msg, const QString &nick);

  QDateTime mReceivedAt;
  QString mMessage;
  QString mNickname;
};

} // namespace Czateria

#endif // MESSAGE_H
