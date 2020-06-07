#include "captchadialog.h"
#include "ui_captchadialog.h"

#include <QDialogButtonBox>
#include <QPushButton>

CaptchaDialog::CaptchaDialog(const QImage &image, QWidget *parent)
    : QDialog(parent), ui(new Ui::CaptchaDialog) {
  ui->setupUi(this);
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  connect(ui->lineEdit, &QLineEdit::textChanged, this, [=](auto &&text) {
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(text.length() == 6);
  });
  ui->label->setPixmap(QPixmap::fromImage(image));
}

CaptchaDialog::~CaptchaDialog() { delete ui; }

QString CaptchaDialog::response() const { return ui->lineEdit->text(); }
