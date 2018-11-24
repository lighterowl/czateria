#ifndef CUSTOMLISTVIEW_H
#define CUSTOMLISTVIEW_H

#include <QListView>

class UserListView : public QListView {
  Q_OBJECT
public:
  explicit UserListView(QWidget *parent = nullptr);

protected:
  void mouseReleaseEvent(QMouseEvent *) override;
signals:
  void mouseMiddleClicked();
};

#endif // CUSTOMLISTVIEW_H
