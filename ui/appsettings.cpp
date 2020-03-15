#include "appsettings.h"

#include <QSettings>

AppSettings::AppSettings() {
  QSettings settings;
  auto variant = settings.value(QLatin1String("auto_pic_save"));
  if (variant.isValid()) {
    savePicturesAutomatically = variant.toBool();
  }
  variant = settings.value(QLatin1String("logins"));
  if (variant.isValid() && variant.type() == QVariant::Hash) {
    auto loginsHash = variant.toHash();
    for (auto it = loginsHash.cbegin(); it != loginsHash.cend(); ++it) {
      logins[it.key()] = it.value().toString();
    }
  }
  settings.beginGroup(QLatin1String("autologin"));
  for (auto idStr : settings.childGroups()) {
    bool ok;
    auto id = idStr.toInt(&ok);
    if (ok) {
      settings.beginGroup(idStr);
      auto user = settings.value(QLatin1String("user"));
      auto pass = settings.value(QLatin1String("pass"));
      if (user.isValid() && pass.isValid()) {
        auto &&loginData = autologin[id];
        loginData.username = user.toString();
        loginData.password = pass.toString();
      }
      settings.endGroup();
    }
  }
  settings.endGroup();
}

AppSettings::~AppSettings() {
  QSettings settings;
  settings.setValue(QLatin1String("auto_pic_save"), savePicturesAutomatically);
  settings.setValue(QLatin1String("logins"), logins);
  settings.beginGroup(QLatin1String("autologin"));
  for (auto it = autologin.cbegin(); it != autologin.cend(); ++it) {
    settings.beginGroup(QString(QLatin1String("%1")).arg(it.key()));
    settings.setValue(QLatin1String("user"), it->username);
    settings.setValue(QLatin1String("pass"), it->password);
    settings.endGroup();
  }
  settings.endGroup();
}
