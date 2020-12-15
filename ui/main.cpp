#include "appsettings.h"
#include "mainwindow.h"
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkProxyFactory>
#include <QStandardPaths>

#include "czatlib/chatsessionlistener.h"

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

struct Logger : public Czateria::ChatSessionListener {
  void onRoomMessage(const Czateria::Room &room, const QString &nickname,
                     const QString &message) override {
    if (mEnableLogging) {
      qInfo() << room.name << nickname << message;
    }
  }
  void onPrivateMessageReceived(const Czateria::Room &room,
                                const QString &nickname,
                                const QString &message) override {
    if (mEnableLogging) {
      qInfo() << room.name << nickname << message;
    }
  }
  void onPrivateMessageSent(const Czateria::Room &room, const QString &nickname,
                            const QString &message) override {
    if (mEnableLogging) {
      qInfo() << room.name << nickname << message;
    }
  }
  void onUserJoined(const Czateria::Room &room,
                    const QString &nickname) override {
    if (mEnableLogging) {
      qInfo() << nickname << "joined" << room.name;
    }
  }
  void onUserLeft(const Czateria::Room &room,
                  const QString &nickname) override {
    if (mEnableLogging) {
      qInfo() << nickname << "left" << room.name;
    }
  }

private:
  static const bool mEnableLogging = true;
};
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
  Logger l;
  MainWindow w(&nam, settings, &l);
  w.show();

  return a.exec();
}
