#include "settingsdialog.h"
#include "ui_chatsettingsform.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::SettingsDialog),
      uiForm(new Ui::ChatSettingsForm) {
  ui->setupUi(this);
  uiForm->setupUi(ui->chatSettingsForm);
}

SettingsDialog::~SettingsDialog() { delete ui; }
