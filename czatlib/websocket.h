#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <QObject>

namespace Czateria {

class WebSocket : public QObject {
  Q_OBJECT
public:
  virtual qint64 sendTextMessage(const QString &message) = 0;
public slots:
  virtual void close() = 0;
  virtual void open(const QUrl &url) = 0;
  virtual QString errorString() const = 0;
signals:
  void textMessageReceived(const QString &message);
  void error(int error);
};

struct WebSocketFactory {
  virtual WebSocket *create(QObject *parent) = 0;
};

} // namespace Czateria

#endif // WEBSOCKET_H
