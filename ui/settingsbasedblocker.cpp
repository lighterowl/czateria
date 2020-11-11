#include "settingsbasedblocker.h"

#include "appsettings.h"

namespace {
bool tryMatch(const QString &subject,
              const QVector<QRegularExpression> &regexes) {
  for (auto &&rgx : regexes) {
    auto m = rgx.match(subject);
    if (m.hasMatch()) {
      return true;
    }
  }
  return false;
}
} // namespace

bool SettingsBasedBlocker::isUserBlocked(const QString &nickname) const {
  return tryMatch(nickname, mSettings.blockedUsers);
}

bool SettingsBasedBlocker::isMessageBlocked(const QString &content) const {
  return tryMatch(content, mSettings.blockedContents);
}
