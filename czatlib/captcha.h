#ifndef CAPTCHA_H
#define CAPTCHA_H

#include <QObject>

class QNetworkAccessManager;
class QImage;

namespace Czateria {

class Captcha : public QObject {
  Q_OBJECT
public:
  Captcha(QNetworkAccessManager *nam, QObject *parent = nullptr);
  void get();

signals:
  void downloaded(const QImage &image, const QString &uid);

private:
  QNetworkAccessManager *const mNAM;

  void onRequestFinished(const QString &content, const QString &callbackName);
  void onImageDownloaded();
};

} // namespace Czateria

#endif // CAPTCHA_H
