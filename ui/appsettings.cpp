#include "appsettings.h"

#include <QSettings>

AppSettings::AppSettings() {
  QSettings settings;
  auto variant = settings.value(QLatin1String("auto_pic_save"));
  if (variant.isValid()) {
    savePicturesAutomatically = variant.toBool();
  }
}

AppSettings::~AppSettings() {
  QSettings settings;
  settings.setValue(QLatin1String("auto_pic_save"), savePicturesAutomatically);
}
