#include "userlistview.h"

#include <QMouseEvent>

UserListView::UserListView(QWidget *parent) : QListView(parent) {}

void UserListView::mouseReleaseEvent(QMouseEvent *ev) {
  QListView::mouseReleaseEvent(ev);
  if (ev->button() == Qt::MiddleButton) {
    emit mouseMiddleClicked();
  }
}
