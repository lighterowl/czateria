#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QHash>
#include <QString>
#include <QVariant>
#include <QSettings>

#include "czatlib/roomlistmodel.h"

namespace Czateria {
struct Room;
}

struct AppSettings : public Czateria::RoomListModel::LoginDataProvider {
  AppSettings();
  ~AppSettings();

  bool savePicturesAutomatically = false;
  QHash<QString, QVariant> logins;

private:
  Czateria::RoomListModel::LoginData
  getAutologin(const Czateria::Room &room) const override;
  void disableAutologin(const Czateria::Room &room) override;
  void enableAutologin(const Czateria::Room &room, QString &&user,
                       QString &&password) override;
  QHash<int, Czateria::RoomListModel::LoginData> mAutologinData;
  QSettings mSettings;
};

#endif // APPSETTINGS_H
