#include "appsettings.h"
#include "mainwindow.h"
#include "qthttpsocket.h"
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkProxyFactory>
#include <QStandardPaths>

namespace {
const QtMessageHandler defaultHandler = qInstallMessageHandler(nullptr);
const bool enableDebugOutput = qEnvironmentVariableIsSet("CZATERIA_DEBUG");

void msgOutput(QtMsgType type, const QMessageLogContext &context,
               const QString &msg) {
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
  qInstallMessageHandler(msgOutput);
  QCoreApplication::setOrganizationName(QLatin1String("xavery"));
  QCoreApplication::setOrganizationDomain(QLatin1String("github.com"));
  QCoreApplication::setApplicationName(QLatin1String("czateria"));
  QApplication a(argc, argv);
  QNetworkProxyFactory::setUseSystemConfiguration(true);
  QtHttpSocketFactory socketFactory;
  auto cache = new QNetworkDiskCache;
  cache->setCacheDirectory(
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
  socketFactory.mNAM.setCache(cache);
  AppSettings settings;
  MainWindow w(&socketFactory, settings);
  w.show();

  return a.exec();
}
