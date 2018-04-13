#ifndef USERLISTMODEL_H
#define USERLISTMODEL_H

#include "user.h"

#include <QAbstractListModel>
#include <QJsonArray>

#include <map>
#include <memory>

class QJsonObject;

namespace Czateria {

class UserListModel : public QAbstractListModel {
  Q_OBJECT
public:
  UserListModel(QObject *parent = nullptr);

  void setUserData(const QJsonArray &userData);
  void setCardData(const QJsonArray &cardData);

  void updateCardData(const QJsonObject &json);
  void setPrivStatus(const QString &nickname, bool hasPrivs);

  void addUsers(const QJsonArray &userData);
  void removeUser(const QString &nickname);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;

private:
  void populateUsers(const QJsonArray &userData, const QJsonArray &cardData);

  std::vector<User> mUsers;

  std::unique_ptr<QJsonArray> mUserDataCache;
  std::unique_ptr<QJsonArray> mCardDataCache;
};

} // namespace Czateria

#endif // USERLISTMODEL_H
