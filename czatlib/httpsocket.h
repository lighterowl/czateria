#ifndef HTTPSOCKET_H
#define HTTPSOCKET_H

#include <QObject>

class QUrl;
class QUrlQuery;

namespace Czateria {

struct HttpSocket : public QObject {
  Q_OBJECT
public:
  virtual QByteArray readAll() = 0;
  virtual QString errorString() const = 0;
  virtual int /* fixme */ error() const = 0;
signals:
  void finished();
};

struct HttpSocketFactory {
  virtual HttpSocket *get(const QUrl &) = 0;
  virtual HttpSocket *getCached(const QUrl &) = 0;
  virtual HttpSocket *post(const QUrl &, const QUrlQuery &) = 0;
};

} // namespace Czateria

#endif // HTTPSOCKET_H
