#include "roomlistmodel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

#include "httpsocket.h"

namespace {
const QStringList model_columns = {QObject::tr("Room name"),
                                   QObject::tr("Users")};
}

namespace Czateria {

RoomListModel::RoomListModel(QObject *parent, HttpSocketFactory *factory)
    : QAbstractTableModel(parent), mSocketFactory(factory), mSocket(nullptr) {
  Q_ASSERT(factory);
}

void RoomListModel::download() {
  mSocket = mSocketFactory->get(
      QUrl(QLatin1String("https://czateria.interia.pl/rooms-list")));
  connect(mSocket, &HttpSocket::finished, this,
          &RoomListModel::onDownloadFinished);
  connect(mSocket, &HttpSocket::downloadError, this,
          &RoomListModel::downloadError);
}

int RoomListModel::rowCount(const QModelIndex &) const { return mRooms.size(); }

int RoomListModel::columnCount(const QModelIndex &) const {
  return model_columns.size();
}

QVariant RoomListModel::data(const QModelIndex &index, int role) const {
  QVariant rv;
  if (role == Qt::DisplayRole) {
    auto &&channel = mRooms[index.row()];
    switch (index.column()) {
    case 0:
      rv = channel.name;
      break;
    case 1:
      rv = channel.num_users;
      break;
    }
  }
  return rv;
}

QVariant RoomListModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    return model_columns[section];
  } else {
    return QAbstractTableModel::headerData(section, orientation, role);
  }
}

QVector<Room> RoomListModel::jsonToChannels(const QJsonArray &arr) {
  QVector<Room> rv;
  rv.reserve(arr.size());
  for (auto &&o : arr) {
    auto obj = o.toObject();
    QJsonValueRef rooms = obj[QLatin1String("rooms")];
    if (!rooms.isArray()) {
      emit replyParseError(QLatin1String("'rooms' not an array"));
      return rv;
    }
    for (QJsonValueRef r : rooms.toArray()) {
      if (!r.isObject()) {
        emit replyParseError(
            QLatin1String("'rooms' array member not an object"));
        return rv;
      }
      QLatin1String channelParseError(nullptr);
      auto c = Room(r.toObject(), channelParseError);
      if (channelParseError.size() != 0) {
        emit replyParseError(channelParseError);
        return rv;
      }
      rv.push_back(c);
    }
  }
  return rv;
}

void RoomListModel::onDownloadFinished() {
  if (mSocket->error()) {
    mSocket->deleteLater();
    mSocket = nullptr;
    return;
  }
  QJsonParseError err;
  auto chanList = QJsonDocument::fromJson(mSocket->readAll(), &err);
  mSocket->deleteLater();
  mSocket = nullptr;

  if (chanList.isNull()) {
    emit jsonError(err);
    return;
  }
  if (!chanList.isArray()) {
    emit replyParseError(QLatin1String("top level object not an array"));
    return;
  }
  beginResetModel();
  mRooms = jsonToChannels(chanList.array());
  endResetModel();
  emit finished();
}

} // namespace Czateria
