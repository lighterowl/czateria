#ifndef TST_CAPTCHA_H
#define TST_CAPTCHA_H

#include "mocks.h"
#include <QBuffer>
#include <QImage>
#include <gtest/gtest.h>

#include <czatlib/captcha.h>

using namespace testing;

TEST(Captcha, Create) {
  MockHttpSocketFactory mock;
  Czateria::Captcha c(&mock);
}

static const QLatin1String captchaUid("83D3727C44A187386A35B17BE076CAE4");

static const QString captchaUrl =
    QString(QLatin1String("https://captcha.interia.pl/%1.jpeg"))
        .arg(captchaUid);

static const QString enigmaJSReply =
    QString(
        QLatin1String(
            R"js(testme( {uid: "%1", url: "%2", type: "1", question: "przepisz kod z obrazka"} ))js"))
        .arg(captchaUid)
        .arg(captchaUrl);

bool isJSRequest(const QUrl &url) {
  return url.path().contains(QLatin1String("getEnigmaJS"));
}

bool isCaptchaImageRequest(const QUrl &url) {
  return url.toString() == captchaUrl;
}

TEST(Captcha, Get) {
  MockHttpSocketFactory mockFactory;
  MockHttpSocket jsRequestSocket;
  MockHttpSocket imgRequestSocket;
  EXPECT_CALL(mockFactory, get(Truly(isJSRequest)))
      .WillOnce(Return(&jsRequestSocket));
  EXPECT_CALL(mockFactory, get(Truly(isCaptchaImageRequest)))
      .WillOnce(Return(&imgRequestSocket));

  Czateria::Captcha c(&mockFactory);
  c.get();
  EXPECT_CALL(jsRequestSocket, readAll)
      .WillOnce(Return(enigmaJSReply.toUtf8()));
  jsRequestSocket.finished();

  QImage image(10, 10, QImage::Format_RGB32);
  image.fill(Qt::yellow);
  QByteArray ba;
  QBuffer buffer(&ba);
  buffer.open(QIODevice::WriteOnly);
  image.save(&buffer, "PNG");

  EXPECT_CALL(imgRequestSocket, readAll).WillOnce(Return(ba));
  bool signalEntered = false;
  QObject::connect(&c, &Czateria::Captcha::downloaded,
                   [&](auto &rcvdImage, auto &uid) {
                     signalEntered = true;
                     ASSERT_EQ(uid, captchaUid);
                     ASSERT_EQ(rcvdImage, image);
                   });
  imgRequestSocket.finished();
  ASSERT_TRUE(signalEntered);
}

#endif // TST_CAPTCHA_H
