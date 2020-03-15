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
    bool isValid() const { return !(username.isEmpty() && username.isEmpty()); }
  };
  using LoginDataHash = QHash<int, LoginData>;
  RoomListModel(QObject *parent, QNetworkAccessManager *nam,
                LoginDataHash &autologinData);
  void download();
  const Room &room(int idx) const { return mRooms[idx]; }

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

signals:
  void finished();
  void downloadError(QNetworkReply::NetworkError error);
  void jsonError(QJsonParseError error);
  void replyParseError(QLatin1String what);

private:
  QVector<Room> mRooms;
  QNetworkAccessManager *const mNAM;
  LoginDataHash &mAutologinData;
  QNetworkReply *mReply;

  QVector<Room> jsonToChannels(const QJsonArray &json);
  void onDownloadFinished();
};
} // namespace Czateria

#endif // RoomListModel_H
