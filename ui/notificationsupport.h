#ifndef NOTIFICATIONSUPPORT_H
#define NOTIFICATIONSUPPORT_H

class MainChatWindow;
class QString;

#include <memory>

struct NotificationSupport {
public:
  virtual void displayNotification(MainChatWindow *chatWin,
                                   const QString &nickname,
                                   const QString &channel) = 0;
  virtual void removeNotification(MainChatWindow *chatWin,
                                  const QString &nickname) = 0;

  static std::unique_ptr<NotificationSupport> msgBox();
  static std::unique_ptr<NotificationSupport> native();
};

#endif // NOTIFICATIONSUPPORT_H
