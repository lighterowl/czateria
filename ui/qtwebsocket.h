#ifndef QTWEBSOCKET_H
#define QTWEBSOCKET_H

#include <czatlib/websocket.h>

struct QtWebSocketFactory : public Czateria::WebSocketFactory {
  Czateria::WebSocket *create(QObject *parent) override;
};

#endif // QTWEBSOCKET_H
