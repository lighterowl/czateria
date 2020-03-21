#include "roomlistmodel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <algorithm>
#include <type_traits>

namespace {
using namespace Czateria;
const QStringList model_columns = {RoomListModel::tr("Room name"),
                                   RoomListModel::tr("Users"),
                                   RoomListModel::tr("Autojoin")};
} // namespace

namespace Czateria {

RoomListModel::RoomListModel(QObject *parent, QNetworkAccessManager *nam,
                             LoginDataProvider &loginProvider)
    : QAbstractTableModel(parent), mNAM(nam), mLoginProvider(loginProvider),
      mReply(nullptr) {
  Q_ASSERT(nam);
}

void RoomListModel::download() {
  mReply = mNAM->get(QNetworkRequest(
      QUrl(QLatin1String("https://czateria.interia.pl/rooms-list"))));
  connect(mReply, &QNetworkReply::finished, this,
          &RoomListModel::onDownloadFinished);
  void (QNetworkReply::*errSignal)(QNetworkReply::NetworkError) =
      &QNetworkReply::error;
  connect(mReply, errSignal, this, &RoomListModel::downloadError);
}

int RoomListModel::rowCount(const QModelIndex &) const { return mRooms.size(); }

int RoomListModel::columnCount(const QModelIndex &) const {
  return model_columns.size();
}

QVariant RoomListModel::data(const QModelIndex &index, int role) const {
  if (role == Qt::DisplayRole) {
    auto &&channel = mRooms[index.row()];
    switch (index.column()) {
    case 0:
      return channel.name;
    case 1:
      return channel.num_users;
    }
  } else if (role == Qt::CheckStateRole && index.column() == 2) {
    auto data = mLoginProvider.getAutologin(mRooms[index.row()]);
    return data.isValid() ? Qt::Checked : Qt::Unchecked;
  }
  return QVariant();
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
  std::sort(std::begin(rv), std::end(rv),
            [](auto &&left, auto &&right) { return left.id < right.id; });
  return rv;
}

void RoomListModel::onDownloadFinished() {
  if (mReply->error() != QNetworkReply::NoError) {
    mReply->deleteLater();
    mReply = nullptr;
    return;
  }
  QJsonParseError err;
  auto chanList = QJsonDocument::fromJson(mReply->readAll(), &err);
  mReply->deleteLater();
  mReply = nullptr;

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
