#include "customlistview.h"

#include <QMouseEvent>

CustomListView::CustomListView(QWidget *parent) : QListView(parent) {}

void CustomListView::mouseReleaseEvent(QMouseEvent *ev) {
  QListView::mouseReleaseEvent(ev);
  if (ev->button() == Qt::MiddleButton) {
    emit mouseMiddleClicked();
  }
}
