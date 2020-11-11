#include "notificationsupport_dbus.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDebug>

#include "mainchatwindow.h"

namespace {
const QLatin1String dbusServiceName("org.freedesktop.Notifications");
const QLatin1String dbusInterfaceName("org.freedesktop.Notifications");
const QLatin1String dbusPath("/org/freedesktop/Notifications");
const QLatin1String acceptPrivConvAction("accept_priv_conv");
const QLatin1String rejectPrivConvAction("reject_priv_conv");
} // namespace

NotificationSupportDBus::NotificationSupportDBus()
    : mSessionBus(QDBusConnection::sessionBus()) {
  if (isServicePresent()) {
    mSessionBus.connect(dbusServiceName, dbusPath, dbusInterfaceName,
                        QLatin1String("ActionInvoked"), this,
                        SLOT(onNotificationActionInvoked(quint32, QString)));
    mSessionBus.connect(dbusServiceName, dbusPath, dbusInterfaceName,
                        QLatin1String("NotificationClosed"), this,
                        SLOT(onNotificationClosed(quint32, quint32)));
  }
}

void NotificationSupportDBus::displayNotification(MainChatWindow *chatWin,
                                                  const QString &nickname,
                                                  const QString &channel) {
  Q_ASSERT(mSessionBus.isConnected());
  auto m = QDBusMessage::createMethodCall(
      dbusServiceName, dbusPath, dbusInterfaceName, QLatin1String("Notify"));
  QVariantList args;
  args.append(QCoreApplication::applicationName());
  args.append(0U);
  args.append(QString());
  args.append(tr("Incoming private conversation"));
  args.append(QString(QLatin1String("<b>%1</b> in room <b>%2</b> wants to "
                                    "start a conversation."))
                  .arg(nickname, channel));
  args.append(QStringList() << acceptPrivConvAction << tr("Accept")
                            << rejectPrivConvAction << tr("Reject"));
  args.append(QMap<QString, QVariant>{
      std::make_pair(QLatin1String("category"), QLatin1String("im.received"))});
  args.append(static_cast<int32_t>(-1));
  m.setArguments(args);
  QDBusReply<quint32> replyMsg = mSessionBus.call(m);
  if (replyMsg.isValid() && replyMsg.value() > 0) {
    mLiveNotifications.insert(replyMsg.value(),
                              NotificationContext{chatWin, nickname});
    connect(chatWin, &QObject::destroyed, this,
            &NotificationSupportDBus::onChatWindowDestroyed,
            Qt::UniqueConnection);
  }
}

bool NotificationSupportDBus::supported() const { return isServicePresent(); }

bool NotificationSupportDBus::isServicePresent() const {
  if (mSessionBus.isConnected()) {
    auto reply = mSessionBus.interface()->isServiceRegistered(dbusServiceName);
    return reply.isValid() && reply.value();
  }
  return false;
}

void NotificationSupportDBus::realRemoveNotification(quint32 notificationId) {
  Q_ASSERT(mSessionBus.isConnected());
  auto m = QDBusMessage::createMethodCall(dbusServiceName, dbusPath,
                                          dbusInterfaceName,
                                          QLatin1String("CloseNotification"));
  QVariantList args;
  args.append(notificationId);
  m.setArguments(args);
  mSessionBus.call(m);
}

void NotificationSupportDBus::onChatWindowDestroyed(QObject *obj) {
  removeNotifications([&](auto &&ctx) { return ctx.chatWin == obj; });
}

void NotificationSupportDBus::onNotificationActionInvoked(
    quint32 notificationId, QString action) {
  auto it = mLiveNotifications.find(notificationId);
  if (it == std::end(mLiveNotifications)) {
    return;
  }
  qDebug() << notificationId << action;
  auto &context = it.value();
  if (action == acceptPrivConvAction) {
    context.chatWin->onPrivateConvNotificationAccepted(context.nickname);
  } else if (action == rejectPrivConvAction) {
    context.chatWin->onPrivateConvNotificationRejected(context.nickname);
  }
}

void NotificationSupportDBus::onNotificationClosed(quint32 id, quint32) {
  mLiveNotifications.remove(id);
  qDebug() << id;
}
