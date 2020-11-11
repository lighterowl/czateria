#ifndef CHATBLOCKER_H
#define CHATBLOCKER_H

#include <QObject>
#include <QString>

namespace Czateria {
class ChatBlocker : public QObject {
  Q_OBJECT
public:
  virtual bool isUserBlocked(const QString &nickname) const = 0;
  virtual bool isMessageBlocked(const QString &content) const = 0;

signals:
  void changed();
};
} // namespace Czateria

#endif // CHATBLOCKER_H
