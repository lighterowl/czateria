#ifndef QTHTTPSOCKET_H
#define QTHTTPSOCKET_H

#include <QNetworkAccessManager>
#include <czatlib/httpsocket.h>

struct QtHttpSocketFactory : public Czateria::HttpSocketFactory {
  Czateria::HttpSocket *get(const QUrl &) override;
  Czateria::HttpSocket *getCached(const QUrl &) override;
  Czateria::HttpSocket *post(const QUrl &, const QUrlQuery &) override;
  QNetworkAccessManager mNAM;
};

#endif // QTHTTPSOCKET_H
