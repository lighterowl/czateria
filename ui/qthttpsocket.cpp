#include "qthttpsocket.h"

#include <QNetworkReply>
#include <QUrlQuery>

namespace {
class QtHttpSocket : public Czateria::HttpSocket {
public:
  QtHttpSocket(QNetworkReply *reply) : mReply(reply) {
    mReply->setParent(this);
    connect(mReply, &QNetworkReply::finished, this, &HttpSocket::finished);
    void (QNetworkReply::*errSignal)(QNetworkReply::NetworkError) =
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        &QNetworkReply::errorOccurred;
#else
        &QNetworkReply::error;
#endif
    connect(mReply, errSignal, [=](auto err) { emit downloadError(err); });
  }
  QByteArray readAll() override { return mReply->readAll(); }
  int error() const override { return mReply->error(); }

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
  request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
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
