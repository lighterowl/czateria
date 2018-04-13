#ifndef CZATERIAROOM_H
#define CZATERIAROOM_H

class QJsonObject;

#include <QString>

namespace Czateria {

struct Room {
  Room() : id(-1), num_users(-1) {}
  Room(const QJsonObject &obj, QLatin1String &error);
  QString name;
  QString port;
  int id;
  int num_users;
};

} // namespace Czateria

#endif // CZATERIAROOM_H
