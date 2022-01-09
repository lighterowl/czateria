#include "notificationsupport_win10.h"

#include "mainchatwindow.h"

#include <WinToast/src/wintoastlib.h>

#include <QCoreApplication>

using namespace WinToastLib;

class NotificationSupportWin10::ToastHandler : public IWinToastHandler {
public:
  ToastHandler(NotificationSupportWin10 &parent, MainChatWindow *chatWin,
               const QString &nickname)
      : mParent(parent), mChatWin(chatWin), mNickname(nickname) {}
  INT64 mToastId = -1;

private:
  void toastActivated() const override {}
  void toastActivated(int actionIndex) const override {
    if (actionIndex == 0) {
      mChatWin->onPrivateConvNotificationAccepted(mNickname);
    } else if (actionIndex == 1) {
      mChatWin->onPrivateConvNotificationRejected(mNickname);
    }
  }
  void toastDismissed(WinToastDismissalReason) const override {
    mParent.mLiveNotifications.remove(mToastId);
  }
  void toastFailed() const override {
    mParent.mLiveNotifications.remove(mToastId);
  }

  NotificationSupportWin10 &mParent;
  MainChatWindow *const mChatWin;
  const QString mNickname;
};

NotificationSupportWin10::NotificationSupportWin10()
    : mWinToast(*WinToast::instance()) {
  auto appname = QCoreApplication::applicationName().toStdWString();
  mWinToast.setAppName(appname);
  mWinToast.setAppUserModelId(WinToast::configureAUMI(
      QCoreApplication::organizationName().toStdWString(), appname));
  mWinToast.setShortcutPolicy(WinToast::SHORTCUT_POLICY_IGNORE);
  mWinToast.initialize();
}

void NotificationSupportWin10::displayNotification(MainChatWindow *chatWin,
                                                   const QString &nickname,
                                                   const QString &channel) {
  auto templ = WinToastTemplate{WinToastTemplate::Text02};
  templ.setFirstLine(
      MainChatWindow::tr("Incoming private conversation").toStdWString());
  templ.setSecondLine(
      QString(QLatin1String("%1 in room %2 wants to start a conversation"))
          .arg(nickname)
          .arg(channel)
          .toStdWString());
  templ.addAction(MainChatWindow::tr("Accept").toStdWString());
  templ.addAction(MainChatWindow::tr("Reject").toStdWString());
  auto handler = std::make_shared<NotificationSupportWin10::ToastHandler>(
      *this, chatWin, nickname);
  auto toastId = mWinToast.showToast(templ, handler);
  if (toastId >= 0) {
    mLiveNotifications.insert(toastId, NotificationContext{chatWin, nickname});
    handler->mToastId = toastId;
    connect(chatWin, &QObject::destroyed, this,
            &NotificationSupportWin10::onChatWindowDestroyed,
            Qt::UniqueConnection);
  }
}

bool NotificationSupportWin10::supported() const {
  return WinToast::isSupportingModernFeatures();
}

void NotificationSupportWin10::onChatWindowDestroyed(QObject *obj) {
  removeNotifications([&](auto &&ctx) { return ctx.chatWin == obj; });
}

void NotificationSupportWin10::realRemoveNotification(qint64 id) {
  mWinToast.hideToast(id);
}
