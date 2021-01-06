#include "appsettings.h"
#include "mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkProxyFactory>
#include <QStandardPaths>

#include "czatlib/chatsessionlistener.h"
#include "czatlib/message.h"

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
  void onRoomMessage(const Czateria::Room &room,
                     const Czateria::Message &message) override {
    if (mEnableLogging) {
      qInfo() << room.name << message.nickname() << message.rawMessage();
    }
  }
  void onPrivateMessageReceived(const Czateria::Room &room,
                                const Czateria::Message &message) override {
    if (mEnableLogging) {
      qInfo() << room.name << message.nickname() << message.rawMessage();
    }
  }
  void onPrivateMessageSent(const Czateria::Room &room,
                            const Czateria::Message &message) override {
    if (mEnableLogging) {
      qInfo() << room.name << message.nickname() << message.rawMessage();
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

  static QString makeLogPath(const QString &input) {
    auto date = QDate::currentDate();
    QString out;
    QRegularExpression rgx(QLatin1String("%[~%uYMDc]"));
    auto it = rgx.globalMatch(input);
    int inpos = 0;
    while (it.hasNext()) {
      auto m = it.next();
      auto matchStart = m.capturedStart();
      if (inpos != m.capturedStart()) {
        out.append(QStringRef(&input, inpos, matchStart - inpos));
      }
      out.append(replaceToken(m.capturedRef().at(1), date));
      inpos = m.capturedEnd();
    }
    if (inpos != input.length()) {
      out.append(QStringRef(&input, inpos, input.length() - inpos));
    }
    return out;
  }

private:
  static const bool mEnableLogging = true;
  static QString replaceToken(QChar token, const QDate &date) {
    switch (token.unicode()) {
    case '%':
      return QLatin1String("%");
    case '~':
      return QDir::homePath();
    case 'u':
      return QLatin1String("kochamkoty69");
    case 'Y':
      return date.toString(QLatin1String("yyyy"));
    case 'M':
      return date.toString(QLatin1String("MM"));
    case 'D':
      return date.toString(QLatin1String("dd"));
    case 'c':
      return QString::fromUtf8("tajny pokój ruchu ośmiu gwiazdek");
    }
    return QString();
  }
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
