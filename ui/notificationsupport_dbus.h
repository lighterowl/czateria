#ifndef NOTIFICATIONSUPPORTDBUS_H
#define NOTIFICATIONSUPPORTDBUS_H

#include "notificationsupport.h"

#include <QDBusConnection>
#include <QHash>
#include <QObject>

class NotificationSupportDBus : public QObject, NotificationSupport {
  Q_OBJECT
public:
  NotificationSupportDBus();
  void displayNotification(MainChatWindow *chatWin, const QString &nickname,
                           const QString &channel) override;
  void removeNotification(MainChatWindow *chatWin,
                          const QString &nickname) override;

private:
  void onChatWindowDestroyed(QObject *);
  void removeNotification(quint32);
  template <typename F> void removeNotifications(F &&);

  QDBusConnection mSessionBus;
  struct NotificationContext {
    MainChatWindow *chatWin;
    QString nickname;
  };
  QHash<quint32, NotificationContext> mLiveNotifications;

private slots:
  void onNotificationActionInvoked(quint32, QString);
  void onNotificationClosed(quint32, quint32);
};

#endif // NOTIFICATIONSUPPORTDBUS_H
