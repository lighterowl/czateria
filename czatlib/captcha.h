#ifndef CAPTCHA_H
#define CAPTCHA_H

#include <QObject>

class QImage;

namespace Czateria {
struct HttpSocketFactory;

class Captcha : public QObject {
  Q_OBJECT
public:
  Captcha(HttpSocketFactory *fac, QObject *parent = nullptr);
  void get();

signals:
  void downloaded(const QImage &image, const QString &uid);

private:
  HttpSocketFactory *const mSocketFactory;

  void onRequestFinished(const QString &content, const QString &callbackName);
  void onImageDownloaded();
};

} // namespace Czateria

#endif // CAPTCHA_H
