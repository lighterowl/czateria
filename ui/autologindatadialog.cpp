#include "autologindatadialog.h"
#include "ui_autologindatadialog.h"

#include <czatlib/roomlistmodel.h>

AutologinDataDialog::AutologinDataDialog(Czateria::RoomListModel &model,
                                         QModelIndex itemIdx, QWidget *parent)
    : QDialog(parent), ui(new Ui::AutologinDataDialog), mItemIdx(itemIdx),
      mModel(model) {
  ui->setupUi(this);
}

AutologinDataDialog::~AutologinDataDialog() { delete ui; }

void AutologinDataDialog::done(int result) {
  if (result == QDialog::Accepted) {
    mModel.enableAutologin(mItemIdx, ui->usernameLineEdit->text(),
                           ui->passwordLineEdit->text());
  } else if (result == QDialog::Rejected) {
    mModel.disableAutologin(mItemIdx);
  }
  QDialog::done(result);
}
