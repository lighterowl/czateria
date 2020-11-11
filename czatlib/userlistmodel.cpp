#include "userlistmodel.h"

#include <QFont>
#include <QJsonObject>
#include <QTextStream>

#include "avatarhandler.h"
#include "chatblocker.h"
#include "chatsession.h"

namespace {
template <typename ForwardIt, typename T>
ForwardIt binary_find(ForwardIt first, ForwardIt last, const T &value) {
  first = std::lower_bound(first, last, value);
  return (first != last && (!(value < *first))) ? first : last;
}

QLatin1String describeSex(Czateria::User::Sex s) {
  using sx = Czateria::User::Sex;
  switch (s) {
  case sx::Male:
    return QLatin1String("Male");
  case sx::Female:
    return QLatin1String("Female");
  case sx::Both:
    return QLatin1String("Both");
  case sx::Unspecified:
    return QLatin1String("Unknown");
  }
  Q_ASSERT(0);
  return QLatin1String();
}

bool sexIsUnspecified(Czateria::User::Sex s) {
  return s == Czateria::User::Sex::Unspecified;
}

QString createToolTip(const Czateria::User &user,
                      const Czateria::AvatarHandler &avatars) {
  QString rv;
  QTextStream s(&rv);
  // return an empty string early instead of an empty HTML body in order to skip
  // showing the tooltip if there's nothing to show.
  if (user.mDescription.isEmpty() && !avatars.hasAvatar(user) &&
      sexIsUnspecified(user.mSex) && sexIsUnspecified(user.mSearchSex) &&
      !user.mBirthDate.isValid()) {
    return QString();
  }
  s << "<html><body><center>";
  if (!user.mDescription.isEmpty()) {
    s << "<i>" << user.mDescription << "</i><br>";
  }
  if (avatars.hasAvatar(user)) {
    auto &&avatar = avatars.getAvatar(user);
    s << QString(
             QLatin1String("<img width=120 height=120 src=\"data:%1;base64, "))
             .arg(avatar.contentType());
    s << QString::fromUtf8(avatar.data.toBase64());
    s << "\">";
  }
  s << "</center><table>";
  if (user.mSex != Czateria::User::Sex::Unspecified) {
    s << "<tr><td>Sex</td><td>" << describeSex(user.mSex) << "</td></tr>";
  }
  if (user.mSearchSex != Czateria::User::Sex::Unspecified) {
    s << "<tr><td>Looking for</td><td>" << describeSex(user.mSearchSex);
    if (user.mAgeFrom || user.mAgeTo) {
      s << " (" << user.mAgeFrom << "-" << user.mAgeTo << ")";
    }
    s << "</tr>";
  }
  if (user.mBirthDate.isValid()) {
    s << "<tr><td>Birthday</td><td>" << user.mBirthDate.toString()
      << "</td></tr>";
  }
  s << "</table></body></html>";
  return rv;
}
} // namespace

namespace Czateria {

UserListModel::UserListModel(const AvatarHandler &avatars,
                             const ChatBlocker &blocker, ChatSession *parent)
    : QAbstractListModel(parent), mSession(*parent), mAvatarHandler(avatars),
      mBlocker(blocker) {}

void UserListModel::setUserData(const QJsonArray &userData) {
  if (mCardDataCache) {
    populateUsers(userData, *mCardDataCache);
  } else {
    mUserDataCache = std::make_unique<QJsonArray>(userData);
  }
}

void UserListModel::setCardData(const QJsonArray &cardData) {
  if (mUserDataCache) {
    populateUsers(*mUserDataCache, cardData);
  } else {
    mCardDataCache = std::make_unique<QJsonArray>(cardData);
  }
}

void UserListModel::populateUsers(const QJsonArray &userData,
                                  const QJsonArray &cardData) {
  beginResetModel();
  mUsers.clear();

  // should be equal, but just to be on the safe side.
  const auto finalIdx = std::min(userData.count(), cardData.count());
  mUsers.reserve(static_cast<unsigned>(finalIdx));
  for (int i = 0; i < finalIdx; ++i) {
    auto usr = User(userData[i].toObject(), cardData[i].toObject());
    if (!mBlocker.isUserBlocked(usr.mLogin)) {
      mUsers.emplace_back(usr);
    }
  }

  std::sort(std::begin(mUsers), std::end(mUsers));
  mUserDataCache.reset();
  mCardDataCache.reset();
  endResetModel();
}

void UserListModel::updateCardData(const QJsonObject &json) {
  Q_ASSERT(json[QLatin1String("code")].toInt() == 184);
  const auto itBegin = std::begin(mUsers);
  const auto itEnd = std::end(mUsers);
  auto it = binary_find(itBegin, itEnd,
                        User(json[QLatin1String("userName")].toString()));
  if (it != itEnd) {
    it->updateCardInfo(json);
    auto row = std::distance(itBegin, it);
    auto modIdx = index(static_cast<int>(row));
    emit dataChanged(modIdx, modIdx, {Qt::ToolTipRole});
  }
}

void UserListModel::setPrivStatus(const QString &nickname, bool hasPrivs) {
  const auto itBegin = std::begin(mUsers);
  const auto itEnd = std::end(mUsers);
  auto it = binary_find(itBegin, itEnd, User(nickname));
  if (it != itEnd) {
    it->mHasPrivs = hasPrivs;
    auto row = std::distance(itBegin, it);
    auto modIdx = index(static_cast<int>(row));
    emit dataChanged(modIdx, modIdx, {Qt::FontRole});
  }
}

void UserListModel::addUsers(const QJsonArray &userData) {
  for (int i = 0; i < userData.size(); ++i) {
    auto user = User(userData[i].toObject());
    if (user.mLogin == mSession.nickname() ||
        !mBlocker.isUserBlocked(user.mLogin)) {
      auto it = std::upper_bound(std::begin(mUsers), std::end(mUsers), user);
      auto row = static_cast<int>(std::distance(std::begin(mUsers), it));
      beginInsertRows(QModelIndex(), row, row);
      mUsers.insert(it, user);
      endInsertRows();
    }
  }
}

void UserListModel::removeUser(const QString &nickname) {
  const auto itBegin = std::begin(mUsers);
  const auto itEnd = std::end(mUsers);
  auto it = binary_find(itBegin, itEnd, User(nickname));
  if (it != itEnd) {
    auto row = static_cast<int>(std::distance(itBegin, it));
    beginRemoveRows(QModelIndex(), row, row);
    mUsers.erase(it);
    endRemoveRows();
  }
}

User *UserListModel::user(const QString &nickname) {
  const auto itBegin = std::begin(mUsers);
  const auto itEnd = std::end(mUsers);
  auto it = binary_find(itBegin, itEnd, User(nickname));
  return it != itEnd ? &(*it) : nullptr;
}

int UserListModel::rowCount(const QModelIndex &) const {
  return static_cast<int>(mUsers.size());
}

QVariant UserListModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  auto &&user = mUsers[static_cast<unsigned>(index.row())];
  switch (role) {
  case Qt::DisplayRole:
    return user.mLogin;
  case Qt::FontRole: {
    QFont font;
    if (user.mHasPrivs) {
      font.setBold(true);
    }
    return font;
  }
  case Qt::ToolTipRole: {
    return createToolTip(user, mAvatarHandler);
  }
  }

  return QVariant();
}

} // namespace Czateria
