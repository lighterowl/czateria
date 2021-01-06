#include "appsettings.h"
#include "mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkProxyFactory>
#include <QStandardPaths>

#include "czatlib/chatsession.h"
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
  void onRoomMessage(const Czateria::ChatSession *session,
                     const Czateria::Message &message) override {
    if (mEnableLogging) {
      qInfo() << session->channel() << message.nickname()
              << message.rawMessage();
    }
  }
  void onPrivateMessageReceived(const Czateria::ChatSession *session,
                                const Czateria::Message &message) override {
    if (mEnableLogging) {
      qInfo() << session->channel() << message.nickname()
              << message.rawMessage();
    }
  }
  void onPrivateMessageSent(const Czateria::ChatSession *session,
                            const Czateria::Message &message) override {
    if (mEnableLogging) {
      qInfo() << session->channel() << message.nickname()
              << message.rawMessage();
    }
  }
  void onUserJoined(const Czateria::ChatSession *session,
                    const QString &nickname) override {
    if (mEnableLogging) {
      qInfo() << nickname << "joined" << session->channel();
    }
  }
  void onUserLeft(const Czateria::ChatSession *session,
                  const QString &nickname) override {
    if (mEnableLogging) {
      qInfo() << nickname << "left" << session->channel();
    }
  }

  static QString makeLogPath(const QString &input,
                             const Czateria::ChatSession *session) {
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
      out.append(replaceToken(m.capturedRef().at(1), date, session));
      inpos = m.capturedEnd();
    }
    if (inpos != input.length()) {
      out.append(QStringRef(&input, inpos, input.length() - inpos));
    }
    return out;
  }

private:
  static const bool mEnableLogging = true;
  static QString replaceToken(QChar token, const QDate &date,
                              const Czateria::ChatSession *session) {
    switch (token.unicode()) {
    case '%':
      return QLatin1String("%");
    case '~':
      return QDir::homePath();
    case 'u':
      return session->nickname();
    case 'Y':
      return date.toString(QLatin1String("yyyy"));
    case 'M':
      return date.toString(QLatin1String("MM"));
    case 'D':
      return date.toString(QLatin1String("dd"));
    case 'c':
      return session->channel();
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
