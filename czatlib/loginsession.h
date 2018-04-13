#ifndef SESSION_H
#define SESSION_H

#include <QObject>
#include <QString>

#include "loginfailreason.h"
#include "room.h"

class QImage;
class QNetworkAccessManager;
class QUrl;
class QUrlQuery;

namespace Czateria {

class LoginSession : public QObject {
  Q_OBJECT
public:
  LoginSession(QNetworkAccessManager *nam, const Room &room,
               QObject *parent = nullptr);

  void login();
  void login(const QString &nickname);
  void login(const QString &nickname, const QString &password);

  void setCaptchaReply(const QString &reply);

  const QString &sessionId() const { return mSessionId; }
  const QString &nickname() const { return mNickname; }
  const Room &room() const { return mLoginRoom; }

signals:
  void loginSuccessful();
  void loginFailed(Czateria::LoginFailReason why,
                   QString loginData = QString());
  void captchaRequired(const QImage &captcha);

private:
  QNetworkAccessManager *const mNAM;
  QString mNickname;
  QString mSessionId;
  QString mCaptchaUid;
  const Room mLoginRoom;

  void onReplyReceived(const QByteArray &content);
  void sendPostData(const QUrl &address, const QUrlQuery &postData);
  QUrlQuery getBasicPostData() const;
};

} // namespace Czateria

#endif // SESSION_H
