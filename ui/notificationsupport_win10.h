#ifndef NOTIFICATIONSUPPORTWIN10_H
#define NOTIFICATIONSUPPORTWIN10_H

#include "notificationsupport.h"

#include <QHash>
#include <QObject>
#include <QString>

namespace WinToastLib {
class WinToast;
}

class NotificationSupportWin10 : public QObject, public NotificationSupport {
  Q_OBJECT
public:
  NotificationSupportWin10();

  void displayNotification(MainChatWindow *chatWin, const QString &nickname,
                           const QString &channel) override;
  void removeNotification(MainChatWindow *chatWin,
                          const QString &nickname) override;
  bool supported() const override;

private:
  class ToastHandler;
  friend class ToastHandler;

  void onChatWindowDestroyed(QObject *);
  void removeNotification(qint64);
  template <typename F> void removeNotifications(F &&);

  WinToastLib::WinToast &mWinToast;
  struct NotificationContext {
    MainChatWindow *chatWin;
    QString nickname;
  };
  QHash<qint64, NotificationContext> mLiveNotifications;
};

#endif // NOTIFICATIONSUPPORTWIN10_H
