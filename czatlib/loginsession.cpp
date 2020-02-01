#include "loginsession.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QUrl>
#include <QUrlQuery>

#include "captcha.h"
#include "httpsocket.h"
#include "room.h"
#include "util.h"

namespace {
bool loginCodeToFailReason(int code, Czateria::LoginFailReason &why) {
  using r = Czateria::LoginFailReason;
  static const std::array<std::tuple<int, r>, 5> codeToFailReason = {
      {{-9, r::BadCaptcha},
       {-5, r::NickRegistered},
       {-6, r::NoSuchUser},
       {-8, r::BadPassword},
       {-10, r::NaughtyNick}}};
  return CzateriaUtil::convert(code, why, codeToFailReason);
}

QString sanitiseNickname(const QString &nickname) {
  auto sanitised = nickname;
  // the website automatically converts spaces to underscores. trying to login
  // with a nickname containing spaces ends in an error message sent by the
  // server after hello.
  sanitised.replace(QLatin1Char(' '), QLatin1Char('_'));
  return sanitised;
}
} // namespace

namespace Czateria {

LoginSession::LoginSession(HttpSocketFactory *factory, QObject *parent)
    : QObject(parent), mSocketFactory(factory) {}

void LoginSession::login(const QString &nickname) {
  mNickname = sanitiseNickname(nickname);
  auto c = new Captcha(mSocketFactory);
  connect(c, &Captcha::downloaded,
          [=](const QImage &image, const QString &uid) {
            c->deleteLater();
            mCaptchaUid = uid;
            emit captchaRequired(image);
          });
  c->get();
}

void LoginSession::login(const Room &room, const QString &nickname,
                         const QString &password) {
  mNickname = sanitiseNickname(nickname);
  mPassword = password;
  auto postData = getBasicPostData(room);
  postData.addQueryItem(QLatin1String("password"), password);
  sendPostData(
      QUrl(QLatin1String("https://czateria-api.interia.pl/scp/user/login")),
      postData);
}

bool LoginSession::restart(const Room &room) {
  if (mPassword.isEmpty()) {
    // only registered users can restart seamlessly due to no captcha being
    // needed
    return false;
  }
  if (!mLoginOngoing) {
    login(room, mNickname, mPassword);
  }
  return true;
}

void LoginSession::setCaptchaReply(const Room &room, const QString &reply) {
  auto postData = getBasicPostData(room);
  postData.addQueryItem(QLatin1String("rulesAccept"), QLatin1String("1"));
  postData.addQueryItem(QLatin1String("captchaUid"), mCaptchaUid);
  postData.addQueryItem(QLatin1String("captchaResponse"), reply);
  postData.addQueryItem(QLatin1String("newCaptcha"), QLatin1String("true"));
  if (mNickname.isEmpty()) {
    postData.addQueryItem(QLatin1String("randomNick"), QLatin1String("1"));
  }
  sendPostData(
      QUrl(QLatin1String("https://czateria-api.interia.pl/scp/user/guest")),
      postData);
}

void LoginSession::onReplyReceived(const QByteArray &data) {
  mLoginOngoing = false;
  qDebug().noquote() << data;
  QJsonParseError err;
  auto json = QJsonDocument::fromJson(data, &err);
  if (json.isNull()) {
    emit loginFailed(LoginFailReason::Unknown);
    return;
  }
  if (!json.isObject()) {
    emit loginFailed(LoginFailReason::Unknown);
    return;
  }

  auto obj = json.object();
  auto login_data = obj[QLatin1String("data")];
  if (obj[QLatin1String("code")].toInt() != 1 ||
      obj[QLatin1String("msg")].toString() != QLatin1String("OK") ||
      obj[QLatin1String("status")].toInt() != 1 || !login_data.isObject()) {
    auto code = obj[QLatin1String("code")].toInt();
    LoginFailReason why = LoginFailReason::Unknown;
    if (!loginCodeToFailReason(code, why)) {
      qInfo() << "Unknown login reply" << code;
    }
    emit loginFailed(why, login_data.toString());
    return;
  }

  mSessionId = login_data.toObject()[QLatin1String("0")].toString();
  emit loginSuccessful();
}

void LoginSession::sendPostData(const QUrl &address,
                                const QUrlQuery &postData) {
  qDebug() << address << postData.toString(QUrl::PrettyDecoded);
  auto postRequest = mSocketFactory->post(address, postData);
  connect(postRequest, &HttpSocket::finished, [=]() {
    auto content = postRequest->readAll();
    postRequest->deleteLater();
    onReplyReceived(content);
  });
  mLoginOngoing = true;
}

QUrlQuery LoginSession::getBasicPostData(const Room &room) const {
  QUrlQuery postData;
  postData.addQueryItem(QLatin1String("nickname"), mNickname);
  postData.addQueryItem(QLatin1String("roomName"), room.name);
  postData.addQueryItem(QLatin1String("roomId"), QString::number(room.id));
  return postData;
}

} // namespace Czateria
