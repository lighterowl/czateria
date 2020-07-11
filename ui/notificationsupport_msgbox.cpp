#include "notificationsupport_msgbox.h"

#include <QAbstractButton>
#include <QMessageBox>

#include "mainchatwindow.h"

void NotificationSupportMsgBox::displayNotification(MainChatWindow *chatWin,
                                                    const QString &nickname,
                                                    const QString &) {
  auto question =
      MainChatWindow::tr("%1 wants to talk in private.\nDo you accept?")
          .arg(nickname);
  auto msgbox = new QMessageBox(
      QMessageBox::Question, MainChatWindow::tr("New private conversation"),
      question, QMessageBox::Yes | QMessageBox::No, chatWin);
  mPendingPrivRequests[nickname] = msgbox;
  msgbox->setDefaultButton(QMessageBox::Yes);
  msgbox->button(QMessageBox::Yes)->setShortcut(QKeySequence());
  msgbox->button(QMessageBox::No)->setShortcut(QKeySequence());
  QObject::connect(msgbox, &QDialog::finished, chatWin, [=](int result) {
    if (result == QMessageBox::Yes) {
      chatWin->onPrivateConvNotificationAccepted(nickname);
    } else {
      chatWin->onPrivateConvNotificationRejected(nickname);
    }
  });
  msgbox->show();
  msgbox->raise();
  msgbox->activateWindow();
}

void NotificationSupportMsgBox::removeNotification(MainChatWindow *,
                                                   const QString &nickname) {
  if (auto msgbox = mPendingPrivRequests.value(nickname, nullptr)) {
    msgbox->reject();
    msgbox->deleteLater();
    mPendingPrivRequests.remove(nickname);
  }
}
