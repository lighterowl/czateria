#include "room.h"

#include <QJsonObject>

namespace Czateria {
Room::Room(const QJsonObject &obj, QLatin1String &error) {
  id = obj[QLatin1String("id")].toInt(-1);
  if (id == -1) {
    error = QLatin1String("room.id not an int");
    return;
  }
  name = obj[QLatin1String("name")].toString();
  if (name.isNull()) {
    error = QLatin1String("room.name not a string");
    return;
  }
  port = obj[QLatin1String("serverPort")].toString();
  if (port.isNull()) {
    error = QLatin1String("room.serverPort not a string");
    return;
  }
  auto users = obj[QLatin1String("usersCount")].toString();
  if (users.isNull()) {
    error = QLatin1String("room.usersCount not a string");
    return;
  }
  bool ok;
  num_users = users.toInt(&ok);
  if (!ok) {
    error = QLatin1String("room.usersCount not parsable as an int");
    return;
  }
}

} // namespace Czateria
