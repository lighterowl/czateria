#include "loginsession.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

#include "captcha.h"
#include "util.h"

namespace {
bool loginCodeToFailReason(int code, Czateria::LoginFailReason &why) {
  using r = Czateria::LoginFailReason;
  static const std::array<std::tuple<int, r>, 6> codeToFailReason = {
      {{-9, r::BadCaptcha},
       {-5, r::NickRegistered},
       {-6, r::NoSuchUser},
       {-8, r::BadPassword},
       {-10, r::NaughtyNick}}};
  return CzateriaUtil::convert(code, why, codeToFailReason);
}
} // namespace

namespace Czateria {

LoginSession::LoginSession(QNetworkAccessManager *nam, const Room &room,
                           QObject *parent)
    : QObject(parent), mNAM(nam), mLoginRoom(room) {}

void LoginSession::login() { login(QString()); }

void LoginSession::login(const QString &nickname) {
  mNickname = nickname;
  auto c = new Captcha(mNAM);
  connect(c, &Captcha::downloaded,
          [=](const QImage &image, const QString &uid) {
            c->deleteLater();
            mCaptchaUid = uid;
            emit captchaRequired(image);
          });
  c->get();
}

void LoginSession::login(const QString &nickname, const QString &password) {
  mNickname = nickname;
  auto postData = getBasicPostData();
  postData.addQueryItem(QLatin1String("password"), password);
  sendPostData(
      QUrl(QLatin1String("https://czateria-api.interia.pl/scp/user/login")),
      postData);
}

void LoginSession::setCaptchaReply(const QString &reply) {
  auto postData = getBasicPostData();
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
      qDebug() << "unknown login reply" << json;
    }
    emit loginFailed(why, login_data.toString());
    return;
  }

  mSessionId = login_data.toObject()[QLatin1String("0")].toString();
  emit loginSuccessful();
}

void LoginSession::sendPostData(const QUrl &address,
                                const QUrlQuery &postData) {
  auto request = QNetworkRequest(address);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QLatin1String("application/x-www-form-urlencoded"));
  auto postRequest =
      mNAM->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
  connect(postRequest, &QNetworkReply::finished, [=]() {
    auto content = postRequest->readAll();
    postRequest->deleteLater();
    onReplyReceived(content);
  });
}

QUrlQuery LoginSession::getBasicPostData() const {
  QUrlQuery postData;
  postData.addQueryItem(QLatin1String("nickname"), mNickname);
  postData.addQueryItem(QLatin1String("roomName"), mLoginRoom.name);
  postData.addQueryItem(QLatin1String("roomId"),
                        QString::number(mLoginRoom.id));
  return postData;
}

} // namespace Czateria
