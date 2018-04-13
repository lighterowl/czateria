#include "mainwindow.h"
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkProxyFactory>

int main(int argc, char **argv) {
  QApplication a(argc, argv);
  QNetworkProxyFactory::setUseSystemConfiguration(true);
  QNetworkAccessManager nam;
  MainWindow w(&nam);
  w.show();

  return a.exec();
}
