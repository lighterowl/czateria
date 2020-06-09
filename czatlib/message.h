#ifndef MESSAGE_H
#define MESSAGE_H

#include "icons.h"
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
  const QString &nickname() const { return mNickname; }
  QString message(IconReplaceMode) const;

private:
  Message(const QString &msg, const QString &nick);

  QDateTime mReceivedAt;
  QString mRawMessage;
  QString mNickname;
};

} // namespace Czateria

#endif // MESSAGE_H
