#ifndef CAPTCHADIALOG_H
#define CAPTCHADIALOG_H

#include <QDialog>

class QNetworkAccessManager;
class QNetworkReply;

namespace Ui {
class CaptchaDialog;
}

class CaptchaDialog : public QDialog {
public:
  explicit CaptchaDialog(const QImage &image, QWidget *parent);
  ~CaptchaDialog();

  QString response() const;

private:
  Ui::CaptchaDialog *ui;
};

#endif // CAPTCHADIALOG_H
