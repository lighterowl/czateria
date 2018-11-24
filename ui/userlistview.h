#ifndef CUSTOMLISTVIEW_H
#define CUSTOMLISTVIEW_H

#include <QListView>

namespace Czateria {
class UserListModel;
class AvatarHandler;
}

class UserListView : public QListView {
  Q_OBJECT
public:
  explicit UserListView(QWidget *parent = nullptr);
  void setUserListModel(Czateria::UserListModel *);
  void setAvatarHandler(Czateria::AvatarHandler *);

protected:
  void mouseReleaseEvent(QMouseEvent *) override;
signals:
  void mouseMiddleClicked();
};

#endif // CUSTOMLISTVIEW_H
