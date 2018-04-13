#ifndef USER_H
#define USER_H

class QJsonObject;

#include <QDate>
#include <QString>

namespace Czateria {

class User {
public:
  // basicInfo comes from code 132, cardInfo from code 183. those two arrays
  // are parallel and objects at the same indices form information about the
  // same user. very peculiar if you ask me, especially seeing how later updates
  // of this information come as single objects.
  User(const QJsonObject &basicInfo, const QJsonObject &cardInfo);
  User(const QString &nickname) : mLogin(nickname) {}
  User(const QJsonObject &basicInfo);

  void updateCardInfo(const QJsonObject &cardInfo);

  enum class Type { Guest, Registered, Admin, SuperAdmin, Honoured };
  enum class Sex { Male, Female, Unspecified };

  bool operator<(const User &u2) const {
    return QString::compare(mLogin, u2.mLogin, Qt::CaseInsensitive) < 0;
  }

  const QString &nickname() const { return mLogin; }
  const QString &description() const { return mDescription; }
  bool hasPrivs() const { return mHasPrivs; }
  void setHasPrivs(bool privs) { mHasPrivs = privs; }

private:
  QString mLogin;
  int mEmotion;
  bool mMobileUser;
  bool mHasPrivs;
  Type mType;
  QString mDescription;
  QString mAvatarId;
  QDate mBirthDate;
  int mUid;
  int mLatitude;
  int mLongitude;
  QString mToken;
  int mAgeFrom;
  int mAgeTo;
  Sex mSex;
  Sex mSearchSex;
};

} // namespace Czateria

#endif // USER_H
