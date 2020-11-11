#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QLineEdit;
class QListWidget;

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
  void onLineEditReturnPressed(QLineEdit *, QListWidget *);
  void accept() override;

  void keyPressEvent(QKeyEvent *) override;
  bool eventFilter(QObject *, QEvent *) override;

  Ui::SettingsDialog *const ui;
  Ui::ChatSettingsForm *const uiForm;
  AppSettings &mAppSettings;
};

#endif // SETTINGSDIALOG_H
