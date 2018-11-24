#include "userlistview.h"

#include "czatlib/avatarhandler.h"
#include "czatlib/userlistmodel.h"

#include <QApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QStyledItemDelegate>
#include <QToolTip>

namespace {
struct UserItemDelegate : public QStyledItemDelegate {
  UserItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
  bool helpEvent(QHelpEvent *event, QAbstractItemView *view,
                 const QStyleOptionViewItem &option,
                 const QModelIndex &index) override {
    // TODO this is probably not the best approach to this, since we're fooling
    // Qt by "deferring" the event processing until the avatar image is actually
    // fetched from the network. this might have unintended consequences, but I
    // couldn't find comes up with any other ideas.
    // alternatively, the site itself mass-fetches all the avatars without
    // toying with loading them lazily. perhaps we could do just that right
    // after receiving the user list from the server, as it would certainly save
    // a lot of hassle...
    if (event->type() == QEvent::ToolTip && index.isValid()) {
      Q_ASSERT(mUserListModel);
      Q_ASSERT(mAvatarHandler);
      auto user = mUserListModel->user(index.data().toString());
      Q_ASSERT(user);
      if (mAvatarHandler->needsDownload(*user)) {
        auto pos = event->pos();
        auto globalPos = event->globalPos();
        mAvatarHandler->downloadAvatar(*user, [=]() {
          QApplication::postEvent(
              view, new QHelpEvent(QEvent::ToolTip, pos, globalPos));
        });
        return false;
      } else {
        return QStyledItemDelegate::helpEvent(event, view, option, index);
      }
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
  }

  Czateria::UserListModel *mUserListModel;
  Czateria::AvatarHandler *mAvatarHandler;

  Q_OBJECT
};
} // namespace

UserListView::UserListView(QWidget *parent) : QListView(parent) {
  setItemDelegate(new UserItemDelegate(this));
}

void UserListView::setUserListModel(Czateria::UserListModel *model) {
  // needed only for the delegate, does not affect the actual model used for the
  // listview.
  static_cast<UserItemDelegate *>(itemDelegate())->mUserListModel = model;
}

void UserListView::setAvatarHandler(Czateria::AvatarHandler *a) {
  static_cast<UserItemDelegate *>(itemDelegate())->mAvatarHandler = a;
}

void UserListView::mouseReleaseEvent(QMouseEvent *ev) {
  QListView::mouseReleaseEvent(ev);
  if (ev->button() == Qt::MiddleButton) {
    emit mouseMiddleClicked();
  }
}

#include "userlistview.moc"
