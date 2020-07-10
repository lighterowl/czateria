#ifndef NOTIFICATIONSUPPORTWIN10_H
#define NOTIFICATIONSUPPORTWIN10_H

#include "notificationsupport.h"

class NotificationSupportWin10 : public NotificationSupport {
public:
  NotificationSupportWin10();

  void displayNotification(MainChatWindow *chatWin, const QString &nickname,
                           const QString &channel) override;
  void removeNotification(MainChatWindow *chatWin,
                          const QString &nickname) override;
};

#endif // NOTIFICATIONSUPPORTWIN10_H
