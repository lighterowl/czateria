#include "captcha.h"

#include <QDateTime>
#include <QDebug>
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>

namespace Czateria {
Captcha::Captcha(QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent), mNAM(nam) {}

void Captcha::get() {
  auto callbackName = QString(QLatin1String("jQuery16202627191567764926_%1"))
                          .arg(QDateTime::currentMSecsSinceEpoch());
  auto requestAddr =
      QString(QLatin1String("https://czateria-api.interia.pl/captcha/"
                            "getEnigmaJS?type=1&ctime=300&callback=%1"))
          .arg(callbackName);
  qDebug() << requestAddr;
  auto captchaRequest = mNAM->get(QNetworkRequest(QUrl(requestAddr)));
  connect(captchaRequest, &QNetworkReply::finished, [=]() {
    auto content = captchaRequest->readAll();
    captchaRequest->deleteLater();
    onRequestFinished(QString::fromUtf8(content), callbackName);
  });
}

void Captcha::onRequestFinished(const QString &content,
                                const QString &callbackName) {
  qDebug().noquote() << content;
  auto js = content.simplified();
  js.remove(QString(QLatin1String("%1(")).arg(callbackName));
  js.remove(QLatin1Char(')'));
  QString url, uid;
  for (auto &&val : js.split(QLatin1String(","))) {
    auto re = QRegularExpression(QLatin1String("\"(.*)\""));
    auto m = re.match(val);
    auto res = m.captured(1);
    if (val.contains(QLatin1String("uid:"))) {
      uid = res;
    } else if (val.contains(QLatin1String("url:"))) {
      url = res;
    }
  }
  qDebug() << url << uid;
  auto imgRequest = mNAM->get(QNetworkRequest(QUrl(url)));
  connect(imgRequest, &QNetworkReply::finished, [=]() {
    auto imgRawContent = imgRequest->readAll();
    imgRequest->deleteLater();
    emit downloaded(QImage::fromData(imgRawContent), uid);
  });
}

} // namespace Czateria
