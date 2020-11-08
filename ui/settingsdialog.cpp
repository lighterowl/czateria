#include "settingsdialog.h"

#include "appsettings.h"
#include "notificationsupport.h"
#include "ui_chatsettingsform.h"
#include "ui_settingsdialog.h"

#include <QStandardItemModel>

SettingsDialog::SettingsDialog(AppSettings &settings, QWidget *parent)
    : QDialog(parent), ui(new Ui::SettingsDialog),
      uiForm(new Ui::ChatSettingsForm), mAppSettings(settings) {
  ui->setupUi(this);
  uiForm->setupUi(ui->chatSettingsForm);

  readSettings();

  auto native = NotificationSupport::native();
  if (!(native && native->supported())) {
    ui->notifStyleComboBox->setCurrentIndex(0);
    auto item =
        static_cast<QStandardItemModel *>(ui->notifStyleComboBox->model())
            ->item(1);
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
  }
}

SettingsDialog::~SettingsDialog() {
  delete ui;
  delete uiForm;
}

void SettingsDialog::readSettings() {
  uiForm->autoAcceptPrivs->setChecked(mAppSettings.autoAcceptPrivs);
  uiForm->autoSavePictures->setChecked(mAppSettings.savePicturesAutomatically);
  uiForm->discardUnaccepted->setChecked(mAppSettings.ignoreUnacceptedMessages);
  uiForm->useEmojiIcons->setChecked(mAppSettings.useEmojiIcons);
  ui->notifStyleComboBox->setCurrentIndex(
      static_cast<int>(mAppSettings.notificationStyle));
}

void SettingsDialog::accept() {
  mAppSettings.autoAcceptPrivs = uiForm->autoAcceptPrivs->isChecked();
  mAppSettings.savePicturesAutomatically =
      uiForm->autoSavePictures->isChecked();
  mAppSettings.ignoreUnacceptedMessages =
      uiForm->discardUnaccepted->isChecked();
  mAppSettings.useEmojiIcons = uiForm->useEmojiIcons->isChecked();
  mAppSettings.notificationStyle = static_cast<AppSettings::NotificationStyle>(
      ui->notifStyleComboBox->currentIndex());

  QDialog::accept();
}
