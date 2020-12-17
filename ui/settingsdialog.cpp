#include "settingsdialog.h"

#include "appsettings.h"
#include "notificationsupport.h"
#include "ui_chatsettingsform.h"
#include "ui_settingsdialog.h"

#include <QKeyEvent>
#include <QRegularExpression>
#include <QStandardItemModel>

namespace {
void fillListWidget(QListWidget *listWidget,
                    const QVector<QRegularExpression> &data) {
  for (auto &&rgx : data) {
    listWidget->addItem(rgx.pattern());
  }
}

QVector<QRegularExpression> saveListWidget(const QListWidget *listWidget) {
  QVector<QRegularExpression> rv;
  rv.reserve(listWidget->count());
  for (int i = 0; i < listWidget->count(); ++i) {
    rv.push_back(QRegularExpression(listWidget->item(i)->text()));
  }
  return rv;
}
} // namespace

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

  connect(ui->blockMsgsLineEdit, &QLineEdit::returnPressed, this, [=]() {
    onLineEditReturnPressed(ui->blockMsgsLineEdit, ui->blockedMsgsList);
  });
  connect(ui->blockUserLineEdit, &QLineEdit::returnPressed, this, [=]() {
    onLineEditReturnPressed(ui->blockUserLineEdit, ui->blockedUsersList);
  });

  ui->blockedMsgsList->installEventFilter(this);
  ui->blockedUsersList->installEventFilter(this);
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

  fillListWidget(ui->blockedMsgsList, mAppSettings.blockedContents);
  fillListWidget(ui->blockedUsersList, mAppSettings.blockedUsers);

  ui->logGeneralCheckBox->setChecked(mAppSettings.logMainChat);
  ui->logJoinsPartsCheckBox->setChecked(mAppSettings.logJoinsParts);
  ui->logGeneralFilePath->setText(mAppSettings.mainChatLogPath);
  ui->logPrivateCheckBox->setChecked(mAppSettings.logPrivs);
  ui->logPrivateFilePath->setText(mAppSettings.privLogPath);
}

void SettingsDialog::onLineEditReturnPressed(QLineEdit *lineEdit,
                                             QListWidget *listWidget) {
  auto str = lineEdit->text();
  QRegularExpression rgx(str);
  if (rgx.isValid()) {
    listWidget->addItem(str);
  } else {
    lineEdit->setToolTip(rgx.errorString());
    lineEdit->setCursorPosition(rgx.patternErrorOffset());
  }
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

  mAppSettings.blockedContents = saveListWidget(ui->blockedMsgsList);
  mAppSettings.blockedUsers = saveListWidget(ui->blockedUsersList);

  mAppSettings.logMainChat = ui->logGeneralCheckBox->isChecked();
  mAppSettings.logJoinsParts = ui->logJoinsPartsCheckBox->isChecked();
  mAppSettings.mainChatLogPath = ui->logGeneralFilePath->text();
  mAppSettings.logPrivs = ui->logPrivateCheckBox->isChecked();
  mAppSettings.privLogPath = ui->logPrivateFilePath->text();

  QDialog::accept();
}

void SettingsDialog::keyPressEvent(QKeyEvent *e) {
  // base QDialog implementation treats enter/return key presses as clicks on
  // the default visible enabled PushButton. in our case, that's the OK button
  // in DialogButtonBox, but this makes it impossible to use
  // QLineEdit::returnPressed, as the events don't even reach it.

  // the base behaviour of treating escape as rejecting the dialog is preserved.
  if (e->matches(QKeySequence::Cancel)) {
    reject();
  } else {
    e->ignore();
  }
}

bool SettingsDialog::eventFilter(QObject *obj, QEvent *ev) {
  if (ev->type() == QEvent::KeyPress &&
      (obj == ui->blockedMsgsList || obj == ui->blockedUsersList)) {
    auto listWidget = static_cast<QListWidget *>(obj);
    auto keyEv = static_cast<QKeyEvent *>(ev);
    if (keyEv->matches(QKeySequence::Delete)) {
      auto item = listWidget->takeItem(listWidget->currentRow());
      delete item;
      return true;
    }
  }
  return QDialog::eventFilter(obj, ev);
}
