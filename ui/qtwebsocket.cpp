#include "qtwebsocket.h"

#include <QWebSocket>

namespace {
class QtWebSocket : public Czateria::WebSocket {
public:
  QtWebSocket(QObject *parent)
      : mSocket(QString(), QWebSocketProtocol::VersionLatest) {
    setParent(parent);
    connect(&mSocket, &QWebSocket::textMessageReceived, this,
            &WebSocket::textMessageReceived);
    void (QWebSocket::*errSig)(QAbstractSocket::SocketError) =
        &QWebSocket::error;
    connect(&mSocket, errSig, [=](auto err) { emit error(err); });
  }

private:
  QWebSocket mSocket;

public:
  qint64 sendTextMessage(const QString &message) override {
    return mSocket.sendTextMessage(message);
  }

  void close() override { mSocket.close(); }
  void open(const QUrl &url) override { mSocket.open(url); }
  QString errorString() const override { return mSocket.errorString(); }
};
} // namespace

Czateria::WebSocket *QtWebSocketFactory::create(QObject *parent) {
  return new QtWebSocket(parent);
}
