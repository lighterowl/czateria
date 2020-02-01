#include "qthttpsocket.h"

#include <QNetworkReply>
#include <QUrlQuery>

namespace {
class QtHttpSocket : public Czateria::HttpSocket {
public:
  QtHttpSocket(QNetworkReply *reply) : mReply(reply) {
    mReply->setParent(this);
    connect(mReply, &QNetworkReply::finished, this, &HttpSocket::finished);
  }
  QByteArray readAll() override { return mReply->readAll(); }
  int error() const override { return mReply->error(); }
  QString errorString() const override { return mReply->errorString(); }

private:
  QNetworkReply *const mReply;
};
} // namespace

Czateria::HttpSocket *QtHttpSocketFactory::get(const QUrl &address) {
  return new QtHttpSocket(mNAM.get(QNetworkRequest(address)));
}

Czateria::HttpSocket *QtHttpSocketFactory::getCached(const QUrl &address) {
  auto request = QNetworkRequest(address);
  request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                       QNetworkRequest::PreferCache);
  return new QtHttpSocket(mNAM.get(request));
}

Czateria::HttpSocket *QtHttpSocketFactory::post(const QUrl &address,
                                                const QUrlQuery &postData) {
  auto request = QNetworkRequest(address);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QLatin1String("application/x-www-form-urlencoded"));
  return new QtHttpSocket(
      mNAM.post(request, postData.toString(QUrl::FullyEncoded).toUtf8()));
}
