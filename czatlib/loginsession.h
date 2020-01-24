#ifndef SESSION_H
#define SESSION_H

#include <QObject>
#include <QString>

#include "loginfailreason.h"

class QImage;
class QNetworkAccessManager;
class QUrl;
class QUrlQuery;

namespace Czateria {
struct Room;
class LoginSession : public QObject {
  Q_OBJECT
public:
  LoginSession(QNetworkAccessManager *nam, QObject *parent = nullptr);

  void login(const QString &nickname = QString());
  void login(const Room &room, const QString &nickname,
             const QString &password);
  bool restart();

  void setCaptchaReply(const Room &room, const QString &reply);

  const QString &sessionId() const { return mSessionId; }
  const QString &nickname() const { return mNickname; }
  void setNickname(const QString &nickname) { mNickname = nickname; }

signals:
  void loginSuccessful();
  void loginFailed(Czateria::LoginFailReason why,
                   QString loginData = QString());
  void captchaRequired(const QImage &captcha);

private:
  QNetworkAccessManager *const mNAM;
  QString mNickname;
  QString mPassword;
  QString mSessionId;
  QString mCaptchaUid;

  void onReplyReceived(const QByteArray &content);
  void sendPostData(const QUrl &address, const QUrlQuery &postData);
  QUrlQuery getBasicPostData(const Room &room) const;
};

} // namespace Czateria

#endif // SESSION_H
