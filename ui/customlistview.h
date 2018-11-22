#ifndef CUSTOMLISTVIEW_H
#define CUSTOMLISTVIEW_H

#include <QListView>

class CustomListView : public QListView {
  Q_OBJECT
public:
  explicit CustomListView(QWidget *parent = nullptr);

protected:
  void mouseReleaseEvent(QMouseEvent *) override;
signals:
  void mouseMiddleClicked();
};

#endif // CUSTOMLISTVIEW_H
