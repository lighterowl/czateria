#ifndef CHATBLOCKER_H
#define CHATBLOCKER_H

#include <QString>

namespace Czateria {
struct ChatBlocker {
  virtual bool isUserBlocked(const QString &nickname) const = 0;
  virtual bool isMessageBlocked(const QString &content) const = 0;
};
} // namespace Czateria

#endif // CHATBLOCKER_H
