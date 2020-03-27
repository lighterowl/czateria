#include "appsettings.h"

AppSettings::AppSettings() {
  auto variant = mSettings.value(QLatin1String("auto_pic_save"));
  if (variant.isValid()) {
    savePicturesAutomatically = variant.toBool();
  }
  variant = mSettings.value(QLatin1String("logins"));
  if (variant.isValid() && variant.type() == QVariant::Hash) {
    auto loginsHash = variant.toHash();
    for (auto it = loginsHash.cbegin(); it != loginsHash.cend(); ++it) {
      logins[it.key()] = it.value().toString();
    }
  }
  mSettings.beginGroup(QLatin1String("autologin"));
  for (auto idStr : mSettings.childGroups()) {
    bool ok;
    auto id = idStr.toInt(&ok);
    if (ok) {
      mSettings.beginGroup(idStr);
      auto user = mSettings.value(QLatin1String("user"));
      auto pass = mSettings.value(QLatin1String("pass"));
      if (user.isValid() && pass.isValid()) {
        auto &&loginData = mAutologinData[id];
        loginData.username = user.toString();
        loginData.password = pass.toString();
      }
      mSettings.endGroup();
    }
  }
  mSettings.endGroup();
}

AppSettings::~AppSettings() {
  mSettings.setValue(QLatin1String("auto_pic_save"), savePicturesAutomatically);
  mSettings.setValue(QLatin1String("logins"), logins);
  mSettings.beginGroup(QLatin1String("autologin"));
  for (auto it = mAutologinData.cbegin(); it != mAutologinData.cend(); ++it) {
    mSettings.beginGroup(QString(QLatin1String("%1")).arg(it.key()));
    mSettings.setValue(QLatin1String("user"), it->username);
    mSettings.setValue(QLatin1String("pass"), it->password);
    mSettings.endGroup();
  }
  mSettings.endGroup();
}

QMultiHash<Czateria::RoomListModel::LoginData, int>
AppSettings::autologinHash() const {
  // the autologin data is stored internally as a hash of channel IDs mapping to
  // LoginData structures, because it's much easier to interact with the Qt
  // models and the QSettings class this way. however, when actually
  // establishing the connections and stuff, it's much easier to have a set of
  // unique logins mapping to lists of channels which are supposed to be joined
  // by using the given login.
  QMultiHash<Czateria::RoomListModel::LoginData, int> rv;
  for (auto it = mAutologinData.begin(); it != mAutologinData.end(); ++it) {
    rv.insert(it.value(), it.key());
  }
  return rv;
}

Czateria::RoomListModel::LoginData
AppSettings::getAutologin(const Czateria::Room &room) const {
  auto it = mAutologinData.find(room.id);
  return it != mAutologinData.end() ? *it
                                    : Czateria::RoomListModel::LoginData();
}

void AppSettings::disableAutologin(const Czateria::Room &room) {
  mAutologinData.remove(room.id);
  mSettings.remove(QString(QLatin1String("autologin/%1")).arg(room.id));
}

void AppSettings::enableAutologin(const Czateria::Room &room, QString &&user,
                                  QString &&password) {
  auto &&loginData = mAutologinData[room.id];
  loginData.username = user;
  loginData.password = password;
}
