#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QHash>
#include <QString>
#include <QVariant>

struct AppSettings {
  AppSettings();
  ~AppSettings();

  bool savePicturesAutomatically = false;
  QHash<QString, QVariant> logins;
};

#endif // APPSETTINGS_H
