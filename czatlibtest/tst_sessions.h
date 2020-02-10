#ifndef TST_SESSIONS_H
#define TST_SESSIONS_H

#include "mocks.h"
#include <gtest/gtest.h>

#include <czatlib/chatsession.h>
#include <czatlib/loginsession.h>

using namespace testing;

TEST(Sessions, Restart) {
  const QString testme = QLatin1String("testme");

  Czateria::Room fakeRoom;
  fakeRoom.name = fakeRoom.port = testme;
  fakeRoom.id = fakeRoom.num_users = 1234;

  MockHttpSocketFactory httpFactory;
  MockHttpSocket loginSocket;
  auto loginSession =
      QSharedPointer<Czateria::LoginSession>::create(&httpFactory);
  EXPECT_CALL(httpFactory, post(_, Truly([&](auto &postData) {
                                  return postData.queryItemValue(QLatin1String(
                                             "nickname")) == testme &&
                                         postData.queryItemValue(QLatin1String(
                                             "password")) == testme;
                                })))
      .WillOnce(Return(&loginSocket));
  loginSession->login(fakeRoom, testme, testme);
  const QByteArray loginReply =
      R"json({
  "data": {
    "0": "381b6baa0bea50d32454ba8551dea2b8",
    "1": "testme",
    "userPerm": 1,
    "loginDate": "07-01-2020,06:31:38",
    "settings": {
      "NICK_COLOUR": "0",
      "MAX_PRIVS": "7"
    }
  },
  "status": 1,
  "code": 1,
  "msg": "OK"
}
)json";
  EXPECT_CALL(loginSocket, readAll).WillOnce(Return(loginReply));
  bool signalEmitted = false;
  QObject::connect(loginSession.data(),
                   &Czateria::LoginSession::loginSuccessful,
                   [&]() { signalEmitted = true; });
  loginSocket.finished();
  ASSERT_TRUE(signalEmitted);

  MockWebSocketFactory wsFactory;
  EXPECT_CALL(wsFactory, create).WillRepeatedly(Return(new MockWebSocket));
}

#endif // TST_SESSIONS_H
