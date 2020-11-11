#ifndef SETTINGSBASEDBLOCKER_H
#define SETTINGSBASEDBLOCKER_H

#include <czatlib/chatblocker.h>

struct AppSettings;

class SettingsBasedBlocker : public Czateria::ChatBlocker {
public:
  SettingsBasedBlocker(const AppSettings &settings) : mSettings(settings) {}

private:
  const AppSettings &mSettings;

  bool isUserBlocked(const QString &nickname) const override;
  bool isMessageBlocked(const QString &content) const override;
};

#endif // SETTINGSBASEDBLOCKER_H
