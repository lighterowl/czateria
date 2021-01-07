#include "filebasedlogger.h"

#include "appsettings.h"

#include <czatlib/chatsession.h>
#include <czatlib/message.h>

#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

namespace {
template <typename F>
QString replaceToken(QChar token, const QDate &date,
                     const Czateria::ChatSession *session, F &&unknownTokenFn) {
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
  return unknownTokenFn(token);
}

template <typename F>
QString makeLogPath(const QString &input, const QRegularExpression &rgx,
                    const Czateria::ChatSession *session, F &&unknownTokenFn) {
  auto date = QDate::currentDate();
  QString out;
  auto it = rgx.globalMatch(input);
  int inpos = 0;
  while (it.hasNext()) {
    auto m = it.next();
    auto matchStart = m.capturedStart();
    if (inpos != m.capturedStart()) {
      out.append(QStringRef(&input, inpos, matchStart - inpos));
    }
    out.append(
        replaceToken(m.capturedRef().at(1), date, session, unknownTokenFn));
    inpos = m.capturedEnd();
  }
  if (inpos != input.length()) {
    out.append(QStringRef(&input, inpos, input.length() - inpos));
  }
  return out;
}

#define COMMON_TOKENS "~%uYMDc"

static const QRegularExpression roomTokensRgx(QLatin1String("%[" COMMON_TOKENS
                                                            "]"));

static const QRegularExpression privTokensRgx(QLatin1String("%[" COMMON_TOKENS
                                                            "p]"));

#undef COMMON_TOKENS

QFileInfo makeRoomLogPath(const QString &path,
                          const Czateria::ChatSession *session) {
  return makeLogPath(path, roomTokensRgx, session,
                     [](auto) { return QString(); });
}

QFileInfo makePrivLogPath(const QString &path,
                          const Czateria::ChatSession *session,
                          const Czateria::Message &msg) {
  return makeLogPath(path, privTokensRgx, session, [&](auto t) {
    if (t.unicode() == 'p') {
      return msg.nickname();
    } else {
      return QString();
    }
  });
}
} // namespace

FileBasedLogger::FileBasedLogger(const AppSettings &settings)
    : mSettings(settings) {}

void FileBasedLogger::onRoomMessage(const Czateria::ChatSession *session,
                                    const Czateria::Message &msg) {
  if (!mSettings.logMainChat) {
    return;
  }
  auto fi = makeRoomLogPath(mSettings.mainChatLogPath, session);
  writeLogEntry(fi, msg.receivedAt(), [&]() {
    return QString(QLatin1String("<%1> %2"))
        .arg(msg.nickname(), msg.rawMessage());
  });
}

void FileBasedLogger::onPrivateMessageReceived(
    const Czateria::ChatSession *session, const Czateria::Message &message) {
  if (!mSettings.logPrivs) {
    return;
  }

  auto fi = makePrivLogPath(mSettings.privLogPath, session, message);
  writeLogEntry(fi, message.receivedAt(), [&]() {
    return QString(QLatin1String("<%1> %2"))
        .arg(message.nickname(), message.rawMessage());
  });
}

void FileBasedLogger::onPrivateMessageSent(const Czateria::ChatSession *session,
                                           const Czateria::Message &message) {
  if (!mSettings.logPrivs) {
    return;
  }
  auto fi = makePrivLogPath(mSettings.privLogPath, session, message);
  writeLogEntry(fi, message.receivedAt(), [&]() {
    return QString(QLatin1String("<%1> %2"))
        .arg(session->nickname(), message.rawMessage());
  });
}

void FileBasedLogger::onUserJoined(const Czateria::ChatSession *session,
                                   const QString &nickname) {
  if (!mSettings.logMainChat || !mSettings.logJoinsParts) {
    return;
  }

  auto fi = makeRoomLogPath(mSettings.mainChatLogPath, session);
  writeLogEntry(fi, QDateTime::currentDateTime(), [&]() {
    return QString(QLatin1String(">>> %1 joined the room")).arg(nickname);
  });
}

void FileBasedLogger::onUserLeft(const Czateria::ChatSession *session,
                                 const QString &nickname) {
  if (!mSettings.logMainChat || !mSettings.logJoinsParts) {
    return;
  }

  auto fi = makeRoomLogPath(mSettings.mainChatLogPath, session);
  writeLogEntry(fi, QDateTime::currentDateTime(), [&]() {
    return QString(QLatin1String("<<< %1 left the room")).arg(nickname);
  });
}

QFile *FileBasedLogger::getOpenFile(const QFileInfo &fileInfo) {
  auto filePath = fileInfo.absoluteFilePath();
  auto it = mOpenFiles.find(filePath);
  if (it != mOpenFiles.end()) {
    return it->second.get();
  }

  QDir().mkpath(fileInfo.absolutePath());
  auto f = std::make_unique<QFile>(filePath);

  if (!f->open(QIODevice::Append | QIODevice::Text)) {
    return nullptr;
  } else {
    auto rv = f.get();
    mOpenFiles[filePath] = std::move(f);
    return rv;
  }
}

template <typename F>
void FileBasedLogger::writeLogEntry(const QFileInfo &fileInfo,
                                    const QDateTime &timestamp,
                                    F &&outputGenFn) {
  if (auto f = getOpenFile(fileInfo)) {
    auto str =
        QString(QLatin1String("[%1] %2\n"))
            .arg(timestamp.toString(QLatin1String("HH:mm:ss")), outputGenFn());
    f->write(str.toUtf8());
    f->flush();
  }
}
