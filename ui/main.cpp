#include "appsettings.h"
#include "mainwindow.h"
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkProxyFactory>
#include <QStandardPaths>

namespace {
QtMessageHandler defaultHandler;

void msgOutput(QtMsgType type, const QMessageLogContext &context,
               const QString &msg) {
  static const bool enableDebugOutput =
      qEnvironmentVariableIsSet("CZATERIA_DEBUG");
  if (type != QtDebugMsg || enableDebugOutput) {
    defaultHandler(type, context, msg);
  }
}
} // namespace

int main(int argc, char **argv) {
  qSetMessagePattern(QLatin1String("[%{time process}] %{type} "
#ifdef QT_MESSAGELOGCONTEXT
                                   "%{file}:%{line} %{function} "
#endif
                                   "%{message}"));
  defaultHandler = qInstallMessageHandler(msgOutput);
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
  MainWindow w(&nam, settings);
  w.show();

  return a.exec();
}
