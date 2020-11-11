#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QHash>
#include <QMultiHash>
#include <QRegularExpression>
#include <QSettings>
#include <QString>
#include <QVariant>

#include "czatlib/roomlistmodel.h"

namespace Czateria {
struct Room;
}

struct AppSettings : public QObject,
                     public Czateria::RoomListModel::LoginDataProvider {
  Q_OBJECT
private:
  QSettings mSettings;

public:
  AppSettings();
  ~AppSettings();

  template <typename T> class Setting {
  public:
    Setting(QSettings &settings, QString &&key, T initialValue);
    ~Setting();
    operator T() const { return mValue; }
    void operator=(const T &value) { mValue = value; }

  private:
    QSettings &mSettings;
    const QString mKey;
    T mValue;
    static T (QVariant::*const mConvFn)() const;
  };

  Setting<bool> useEmojiIcons;
  Setting<bool> savePicturesAutomatically;
  Setting<bool> ignoreUnacceptedMessages;
  Setting<bool> autoAcceptPrivs;
  enum class NotificationStyle { MessageBox, Native };
  Q_ENUM(NotificationStyle)
  NotificationStyle notificationStyle = NotificationStyle::MessageBox;

  QHash<QString, QVariant> logins;
  QMultiHash<Czateria::RoomListModel::LoginData, int> autologinHash() const;

  QVector<QRegularExpression> blockedUsers;
  QVector<QRegularExpression> blockedContents;

private:
  Czateria::RoomListModel::LoginData
  getAutologin(const Czateria::Room &room) const override;
  void disableAutologin(const Czateria::Room &room) override;
  void enableAutologin(const Czateria::Room &room, QString &&user,
                       QString &&password) override;
  QHash<int, Czateria::RoomListModel::LoginData> mAutologinData;
};

#endif // APPSETTINGS_H
