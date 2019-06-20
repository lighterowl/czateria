#include "appsettings.h"
#include "mainwindow.h"
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkProxyFactory>
#include <QStandardPaths>

int main(int argc, char **argv) {
  QCoreApplication::setOrganizationName(QLatin1String("xavery"));
  QCoreApplication::setOrganizationDomain(QLatin1String("github.com"));
  QCoreApplication::setApplicationName(QLatin1String("czateria"));
  QApplication a(argc, argv);
  QNetworkProxyFactory::setUseSystemConfiguration(true);
  QNetworkAccessManager nam;
  auto cache = new QNetworkDiskCache;
  cache->setCacheDirectory(
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
  nam.setCache(cache);
  AppSettings settings;
  auto w = new MainWindow(&nam, settings);
  w->show();

  return a.exec();
}
