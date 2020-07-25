#ifndef NOTIFICATIONSUPPORT_NATIVE_H
#define NOTIFICATIONSUPPORT_NATIVE_H

#include "notificationsupport.h"

#include <QHash>
#include <QString>

class QObject;

template <typename Derived, typename NativeIdType>
class NotificationSupportNative : public NotificationSupport {
  void removeNotification(MainChatWindow *chatWin,
                          const QString &nickname) final override {
    removeNotifications([&](auto &&ctx) {
      return ctx.chatWin == chatWin && ctx.nickname == nickname;
    });
  }

protected:
  struct NotificationContext {
    MainChatWindow *chatWin;
    QString nickname;
  };
  QHash<NativeIdType, NotificationContext> mLiveNotifications;
  template <typename F> void removeNotifications(F &&f) {
    const auto it_end = std::end(mLiveNotifications);
    for (auto it = std::begin(mLiveNotifications); it != it_end; ++it) {
      if (f(it.value())) {
        static_cast<Derived *>(this)->realRemoveNotification(it.key());
      }
    }
  }
};

#endif // NOTIFICATIONSUPPORT_NATIVE_H
