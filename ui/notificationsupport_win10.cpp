#include "notificationsupport_win10.h"

NotificationSupportWin10::NotificationSupportWin10() {}

void NotificationSupportWin10::displayNotification(MainChatWindow *,
                                                   const QString &,
                                                   const QString &) {}

void NotificationSupportWin10::removeNotification(MainChatWindow *,
                                                  const QString &) {}

bool NotificationSupportWin10::supported() const { return false; }
