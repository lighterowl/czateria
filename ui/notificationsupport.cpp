#include "notificationsupport.h"
#include "notificationsupport_msgbox.h"

std::unique_ptr<NotificationSupport> NotificationSupport::msgBox() {
  return std::make_unique<NotificationSupportMsgBox>();
}

#ifdef QT_DBUS_LIB
#include "notificationsupport_dbus.h"
std::unique_ptr<NotificationSupport> NotificationSupport::native() {
  return std::make_unique<NotificationSupportDBus>();
}
#elif defined(Q_OS_WIN)
#include "notificationsupport_win10.h"
std::unique_ptr<NotificationSupport> NotificationSupport::native() {
  return std::make_unique<NotificationSupportWin10>();
}
#else
std::unique_ptr<NotificationSupport> NotificationSupport::native() {
  return nullptr;
}
#endif
