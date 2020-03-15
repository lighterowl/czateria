#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QHash>
#include <QString>
#include <QVariant>

#include "czatlib/roomlistmodel.h"

struct AppSettings {
  AppSettings();
  ~AppSettings();

  bool savePicturesAutomatically = false;
  QHash<QString, QVariant> logins;
  Czateria::RoomListModel::LoginDataHash autologin;
};

#endif // APPSETTINGS_H
