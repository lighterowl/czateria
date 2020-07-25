#ifndef NOTIFICATIONSUPPORTWIN10_H
#define NOTIFICATIONSUPPORTWIN10_H

#include "notificationsupport_native.h"

#include <QObject>

namespace WinToastLib {
class WinToast;
}

class NotificationSupportWin10
    : public QObject,
      public NotificationSupportNative<NotificationSupportWin10, qint64> {
  Q_OBJECT
public:
  NotificationSupportWin10();

  void displayNotification(MainChatWindow *chatWin, const QString &nickname,
                           const QString &channel) override;
  bool supported() const override;
  void realRemoveNotification(qint64);

private:
  class ToastHandler;
  friend class ToastHandler;

  void onChatWindowDestroyed(QObject *);
  template <typename F> void removeNotifications(F &&);

  WinToastLib::WinToast &mWinToast;
};

#endif // NOTIFICATIONSUPPORTWIN10_H
