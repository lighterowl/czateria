#ifndef RoomListModel_H
#define RoomListModel_H

#include <QAbstractTableModel>
#include <QJsonParseError>
#include <QNetworkReply>
#include <QVector>

#include "room.h"

class QNetworkAccessManager;
class QNetworkReply;
class QJsonArray;
class QJsonObject;

namespace Czateria {
class RoomListModel : public QAbstractTableModel {
  Q_OBJECT
public:
  struct LoginData {
    QString username;
    QString password;
    bool isValid() const { return !(username.isEmpty() && password.isEmpty()); }
    bool operator==(const LoginData &other) const {
      return username == other.username && password == other.password;
    }
  };
  struct LoginDataProvider {
    virtual LoginData getAutologin(const Czateria::Room &index) const = 0;
    virtual void disableAutologin(const Czateria::Room &index) = 0;
    virtual void enableAutologin(const Czateria::Room &room, QString &&user,
                                 QString &&password) = 0;
  };

  RoomListModel(QObject *parent, QNetworkAccessManager *nam,
                LoginDataProvider &loginProvider);
  void download();
  const Room &room(int idx) const { return mRooms[idx]; }
  const Room *roomFromId(int roomId) const;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

  void disableAutologin(const QModelIndex &index) {
    mLoginProvider.disableAutologin(room(index.row()));
    emit dataChanged(index, index, {Qt::CheckStateRole});
  }
  void enableAutologin(const QModelIndex &index, QString &&user,
                       QString &&password) {
    mLoginProvider.enableAutologin(room(index.row()), std::move(user),
                                   std::move(password));
    emit dataChanged(index, index, {Qt::CheckStateRole});
  }
  LoginData getAutologin(const QModelIndex &index) const {
    return mLoginProvider.getAutologin(room(index.row()));
  }

signals:
  void finished();
  void downloadError(QNetworkReply::NetworkError error);
  void jsonError(QJsonParseError error);
  void replyParseError(QLatin1String what);

private:
  QVector<Room> mRooms;
  QNetworkAccessManager *const mNAM;
  LoginDataProvider &mLoginProvider;
  QNetworkReply *mReply;

  QVector<Room> jsonToChannels(const QJsonArray &json);
  void onDownloadFinished();
};
inline uint qHash(const Czateria::RoomListModel::LoginData &data) {
  return qHash(data.username) ^ qHash(data.password);
}
} // namespace Czateria

#endif // RoomListModel_H
