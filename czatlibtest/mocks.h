#ifndef MOCKS_H
#define MOCKS_H

#include <QUrl>
#include <QUrlQuery>
#include <czatlib/httpsocket.h>
#include <czatlib/websocket.h>

#include <gmock/gmock.h>

struct MockWebSocket : public Czateria::WebSocket {
  MOCK_METHOD(qint64, sendTextMessage, (const QString &), (override));
  MOCK_METHOD(void, close, (), (override));
  MOCK_METHOD(void, open, (const QUrl &), (override));
  MOCK_METHOD(QString, errorString, (), (const, override));
};

struct MockWebSocketFactory : public Czateria::WebSocketFactory {
  MOCK_METHOD(Czateria::WebSocket *, create, (QObject *), (override));
};

struct MockHttpSocket : public Czateria::HttpSocket {
  MOCK_METHOD(QByteArray, readAll, (), (override));
  MOCK_METHOD(int, error, (), (const, override));
};

struct MockHttpSocketFactory : public Czateria::HttpSocketFactory {
  MOCK_METHOD(Czateria::HttpSocket *, get, (const QUrl &), (override));
  MOCK_METHOD(Czateria::HttpSocket *, getCached, (const QUrl &), (override));
  MOCK_METHOD(Czateria::HttpSocket *, post, (const QUrl &, const QUrlQuery &),
              (override));
};

#endif // MOCKS_H
