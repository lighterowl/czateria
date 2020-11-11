#ifndef USERLISTMODEL_H
#define USERLISTMODEL_H

#include "user.h"

#include <QAbstractListModel>
#include <QJsonArray>

#include <map>
#include <memory>

class QJsonObject;

namespace Czateria {

class AvatarHandler;
class ChatBlocker;
class ChatSession;

class UserListModel : public QAbstractListModel {
  Q_OBJECT
public:
  UserListModel(const AvatarHandler &avatars, const ChatBlocker &blocker,
                ChatSession *parent);

  void setUserData(const QJsonArray &userData);
  void setCardData(const QJsonArray &cardData);

  void updateCardData(const QJsonObject &json);
  void setPrivStatus(const QString &nickname, bool hasPrivs);

  void addUsers(const QJsonArray &userData);
  void removeUser(const QString &nickname);
  User *user(const QString &nickname);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;

private:
  void populateUsers(const QJsonArray &userData, const QJsonArray &cardData);
  void onBlockerChanged();
  std::vector<User>::iterator
  removeUserInternal(std::vector<User>::iterator it);

  std::vector<User> mUsers;

  std::unique_ptr<QJsonArray> mUserDataCache;
  std::unique_ptr<QJsonArray> mCardDataCache;

  const ChatSession &mSession;
  const AvatarHandler &mAvatarHandler;
  const ChatBlocker &mBlocker;
};

} // namespace Czateria

#endif // USERLISTMODEL_H
