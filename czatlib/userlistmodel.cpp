#include "userlistmodel.h"

#include <QFont>
#include <QJsonObject>

namespace {
template <typename ForwardIt, typename T>
ForwardIt binary_find(ForwardIt first, ForwardIt last, const T &value) {
  first = std::lower_bound(first, last, value);
  return (first != last && (!(value < *first))) ? first : last;
}
} // namespace

namespace Czateria {

UserListModel::UserListModel(QObject *parent) : QAbstractListModel(parent) {}

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
    mUsers.push_back(User(userData[i].toObject(), cardData[i].toObject()));
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
    it->setHasPrivs(hasPrivs);
    auto row = std::distance(itBegin, it);
    auto modIdx = index(static_cast<int>(row));
    emit dataChanged(modIdx, modIdx, {Qt::FontRole});
  }
}

void UserListModel::addUsers(const QJsonArray &userData) {
  for (int i = 0; i < userData.size(); ++i) {
    auto user = User(userData[i].toObject());
    auto it = std::upper_bound(std::begin(mUsers), std::end(mUsers), user);
    auto row = static_cast<int>(std::distance(std::begin(mUsers), it));
    beginInsertRows(QModelIndex(), row, row);
    mUsers.insert(it, user);
    endInsertRows();
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
    return user.nickname();
  case Qt::FontRole: {
    QFont font;
    if (user.hasPrivs()) {
      font.setBold(true);
    }
    return font;
  }
  case Qt::ToolTipRole: {
    return user.description();
  }
  }

  return QVariant();
}

} // namespace Czateria
