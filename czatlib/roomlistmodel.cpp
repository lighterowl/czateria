#include "roomlistmodel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <algorithm>
#include <type_traits>

namespace {
const QStringList model_columns = {QObject::tr("Room name"),
                                   QObject::tr("Users")};
}

namespace Czateria {

RoomListModel::RoomListModel(QObject *parent, QNetworkAccessManager *nam)
    : QAbstractTableModel(parent), mNAM(nam), mReply(nullptr) {
  Q_ASSERT(nam);
}

void RoomListModel::download() {
  beginResetModel();
  mRooms.clear();
  mReply = mNAM->get(QNetworkRequest(
      QUrl(QLatin1String("https://czateria.interia.pl/rooms-list"))));
  connect(mReply, &QNetworkReply::finished, this,
          &RoomListModel::onDownloadFinished);
  void (QNetworkReply::*errSignal)(QNetworkReply::NetworkError) =
      &QNetworkReply::error;
  connect(mReply, errSignal, this, &RoomListModel::downloadError);
  endResetModel();
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

template <typename ContainerT, typename MemberT, typename SortFn>
void sortByMember(ContainerT &v, MemberT ContainerT::value_type::*p, SortFn x) {
  std::stable_sort(std::begin(v), std::end(v),
                   [=](auto &&a, auto &&b) { return x(a.*p, b.*p); });
}

template <typename SortFn>
void sortByColumn(QVector<Czateria::Room> &v, int col, SortFn x) {
  using room = std::remove_reference<decltype(v)>::type::value_type;
  switch (col) {
  case 0:
    sortByMember(v, &room::name, x);
    break;
  case 1:
    sortByMember(v, &room::num_users, x);
    break;
  default:
    Q_ASSERT(0 && "unknown column");
  }
}

void RoomListModel::sort(int column, Qt::SortOrder order) {
  emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(),
                              QAbstractItemModel::VerticalSortHint);
  switch (order) {
  case Qt::AscendingOrder:
    sortByColumn(mRooms, column, std::less<>{});
    break;
  case Qt::DescendingOrder:
    sortByColumn(mRooms, column, std::greater<>{});
    break;
  }
  emit layoutChanged(QList<QPersistentModelIndex>(),
                     QAbstractItemModel::VerticalSortHint);
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
