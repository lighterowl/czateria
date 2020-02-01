#ifndef RoomListModel_H
#define RoomListModel_H

#include <QAbstractTableModel>
#include <QJsonParseError>
#include <QVector>

#include "room.h"

class QJsonArray;
class QJsonObject;

namespace Czateria {
struct HttpSocketFactory;
class HttpSocket;
class RoomListModel : public QAbstractTableModel {
  Q_OBJECT
public:
  RoomListModel(QObject *parent, HttpSocketFactory *factory);
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
  void downloadError(int error);
  void jsonError(QJsonParseError error);
  void replyParseError(QLatin1String what);

private:
  QVector<Room> mRooms;
  HttpSocketFactory *mSocketFactory;
  HttpSocket *mSocket;

  QVector<Room> jsonToChannels(const QJsonArray &json);
  void onDownloadFinished();
};
} // namespace Czateria

#endif // RoomListModel_H
