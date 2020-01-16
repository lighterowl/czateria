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
}

AppSettings::~AppSettings() {
  QSettings settings;
  settings.setValue(QLatin1String("auto_pic_save"), savePicturesAutomatically);
  settings.setValue(QLatin1String("logins"), logins);
}
