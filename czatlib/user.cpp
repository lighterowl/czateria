#include "user.h"

#include <QJsonObject>

namespace {
Czateria::User::Type permToUserType(int perm) {
  using t = Czateria::User::Type;
  switch (perm) {
  case 0:
    return t::Guest;
  case 1:
  case 2:
    return t::Registered;
  case 3:
    return t::Admin;
  case 4:
    return t::SuperAdmin;
  case 5:
    return t::Honoured;
  }
  return t::Guest;
}

Czateria::User::Sex sexToUserSex(const QString &sex) {
  using sx = Czateria::User::Sex;
  if (sex == QLatin1String("M")) {
    return sx::Male;
  } else if (sex == QLatin1String("F")) {
    return sx::Female;
  } else {
    return sx::Unspecified;
  }
}
} // namespace

namespace Czateria {

User::User(const QJsonObject &basicInfo, const QJsonObject &cardInfo)
    : User(basicInfo) {
  updateCardInfo(cardInfo);
}

User::User(const QJsonObject &basicInfo)
    : mLogin(basicInfo[QLatin1String("login")].toString()),
      mEmotion(basicInfo[QLatin1String("emotion")].toInt()),
      mMobileUser(basicInfo[QLatin1String("isMobileUser")].toBool()),
      mHasPrivs(basicInfo[QLatin1String("privs")].toInt() > 0),
      mType(permToUserType(basicInfo[QLatin1String("perm")].toInt())) {}

/* TODO : avatars are downloaded from one of these two addresses :
 * https://qan.interia.pl/chat/applet/chat_resources/images/avatars/defaults/17.jpg
 * or
 * https://qan.interia.pl/chat/applet/chat_resources/images/avatars/users/15280697_1495985497508.jpg
 * looks like both can be accessed without any extra authentication
 */

void User::updateCardInfo(const QJsonObject &cardInfo) {
  mDescription = cardInfo[QLatin1String("description")].toString();
  mAvatarId = cardInfo[QLatin1String("avatarId")].toString();
  mBirthDate = QDate::fromString(cardInfo[QLatin1String("bornDate")].toString(),
                                 QLatin1String("dd-MM-yyyy"));
  mUid = cardInfo[QLatin1String("uid")].toInt();
  mLatitude = cardInfo[QLatin1String("lat")].toInt();
  mLongitude = cardInfo[QLatin1String("lon")].toInt();
  mToken = cardInfo[QLatin1String("token")].toString();
  mAgeFrom = cardInfo[QLatin1String("searchAgeFrom")].toInt();
  mAgeTo = cardInfo[QLatin1String("searchAgeTo")].toInt();
  mSex = sexToUserSex(cardInfo[QLatin1String("sex")].toString());
  mSearchSex = sexToUserSex(cardInfo[QLatin1String("searchSex")].toString());
}

} // namespace Czateria
