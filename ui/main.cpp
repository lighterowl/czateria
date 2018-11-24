#include "mainwindow.h"
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkProxyFactory>
#include <QStandardPaths>

int main(int argc, char **argv) {
  QApplication a(argc, argv);
  QNetworkProxyFactory::setUseSystemConfiguration(true);
  QNetworkAccessManager nam;
  auto cache = new QNetworkDiskCache;
  cache->setCacheDirectory(
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
  nam.setCache(cache);
  MainWindow w(&nam);
  w.show();

  return a.exec();
}
