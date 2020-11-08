#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
class ChatSettingsForm;
} // namespace Ui

struct AppSettings;

class SettingsDialog : public QDialog {
  Q_OBJECT

public:
  SettingsDialog(AppSettings &settings, QWidget *parent = nullptr);
  ~SettingsDialog();

private:
  void readSettings();
  void accept() override;

  Ui::SettingsDialog *const ui;
  Ui::ChatSettingsForm *const uiForm;
  AppSettings &mAppSettings;
};

#endif // SETTINGSDIALOG_H
