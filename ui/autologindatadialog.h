#ifndef AUTOLOGINDATADIALOG_H
#define AUTOLOGINDATADIALOG_H

#include <QDialog>
#include <QModelIndex>

namespace Ui {
class AutologinDataDialog;
}

namespace Czateria {
class RoomListModel;
}

class AutologinDataDialog : public QDialog {
  Q_OBJECT

public:
  AutologinDataDialog(Czateria::RoomListModel &model, QModelIndex itemIdx,
                      QWidget *parent = nullptr);
  ~AutologinDataDialog();

private:
  void done(int) override;

  Ui::AutologinDataDialog *const ui;
  const QModelIndex mItemIdx;
  Czateria::RoomListModel &mModel;
};

#endif // AUTOLOGINDATADIALOG_H
