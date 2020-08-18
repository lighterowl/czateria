#ifndef NOTIFICATIONSUPPORTDBUS_H
#define NOTIFICATIONSUPPORTDBUS_H

#include "notificationsupport_native.h"

#include <QDBusConnection>
#include <QObject>

class NotificationSupportDBus
    : public QObject,
      public NotificationSupportNative<NotificationSupportDBus, quint32> {
  Q_OBJECT
public:
  NotificationSupportDBus();
  void displayNotification(MainChatWindow *chatWin, const QString &nickname,
                           const QString &channel) override;
  bool supported() const override;
  void realRemoveNotification(quint32 notificationId);

private:
  void onChatWindowDestroyed(QObject *obj);
  bool isServicePresent() const;

  QDBusConnection mSessionBus;

private slots:
  void onNotificationActionInvoked(quint32, QString);
  void onNotificationClosed(quint32, quint32);
};

#endif // NOTIFICATIONSUPPORTDBUS_H
