#ifndef NOTIFICATIONSUPPORTMSGBOX_H
#define NOTIFICATIONSUPPORTMSGBOX_H

#include "notificationsupport.h"

#include <QHash>

class QMessageBox;

struct NotificationSupportMsgBox : public NotificationSupport {
  void displayNotification(MainChatWindow *chatWin, const QString &nickname,
                           const QString &channel) override;
  void removeNotification(MainChatWindow *chatWin,
                          const QString &nickname) override;
  bool supported() const override { return true; }

private:
  QHash<QString, QMessageBox *> mPendingPrivRequests;
};

#endif // NOTIFICATIONSUPPORTMSGBOX_H
