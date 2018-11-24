#ifndef AVATARHANDLER_H
#define AVATARHANDLER_H

#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "user.h"

class QNetworkAccessManager;

namespace Czateria {

class AvatarHandler {
public:
  struct Avatar {
    enum class Format { PNG, JPG };
    Format format;
    QByteArray data;
    QLatin1String contentType() const {
      switch (format) {
      case Format::PNG:
        return QLatin1String("image/png");
      case Format::JPG:
        return QLatin1String("image/jpg");
      }
      Q_ASSERT(0);
      return QLatin1String();
    }
  };

  explicit AvatarHandler(QNetworkAccessManager *nam) : mNAM(nam) {}

  template <typename FetchedFnT>
  void downloadAvatar(const User &user, FetchedFnT fetchedFn) {
    Q_ASSERT(needsDownload(user));
    QString address;
    auto &&avatarId = user.mAvatarId;
    Avatar::Format fmt;

    if (avatarId.length() <= 2) {
      static const std::array<QLatin1String, 37> defaults = {
          QLatin1String("0007CY94U0F5I2T0"), QLatin1String("0007CY7OUHUUOVC4"),
          QLatin1String("0007CY83K51QMXUH"), QLatin1String("0007DH2IBF8WULD5"),
          QLatin1String("0007CY880T6DETYC"), QLatin1String("0007CY896GTRBDWE"),
          QLatin1String("0007CY8AB4O5MPQJ"), QLatin1String("0007CY8ETOWITNRX"),
          QLatin1String("0007CY8HQ6P0VIUT"), QLatin1String("0007CY8J91F1T56G"),
          QLatin1String("0007CY8LX62SRH5A"), QLatin1String("0007CY8M5HYD0WLA"),
          QLatin1String("0007CY8O8WWUJ68N"), QLatin1String("0007CY8RCVFFXGEM"),
          QLatin1String("0007CY8SH1QN17WX"), QLatin1String("0007CY8UYINS2U56"),
          QLatin1String("0007CY8VMJWVQ0VD"), QLatin1String("0007CY8W73V7D82N"),
          QLatin1String("0007CY8X5C98EDM0"), QLatin1String("0007CY8YMHLK1G8S"),
          QLatin1String("0007CY8ZN3ERO4H6"), QLatin1String("0007CY90CSV4YFBB"),
          QLatin1String("0007CY91FMWMGGW8"), QLatin1String("0007CY92FRN013EP"),
          QLatin1String("0007CY932SSKHP6N"), QLatin1String("0007CY86EN2ERNYY"),
          QLatin1String("0007CY875CAE7ULI"), QLatin1String("0007CY8BMBQRMVIX"),
          QLatin1String("0007CY8CAFA4SMQU"), QLatin1String("0007CY8DBK1DQO6V"),
          QLatin1String("0007CY8GTI3U1LWT"), QLatin1String("0007CY8IOIJ7D81G"),
          QLatin1String("0007CY8K1T3YKWXY"), QLatin1String("0007CY8NA1ACWK3R"),
          QLatin1String("0007CY8P6FNXQXFE"), QLatin1String("0007CY8QFXE5MNF6"),
          QLatin1String("0007CY8TF1U7FIEE")};
      address.append(QLatin1String("https://i.iplsc.com/-/"));
      bool ok;
      auto avatar_id = avatarId.toUInt(&ok);
      if (!ok || avatar_id >= defaults.size()) {
        return;
      }
      address.append(defaults[avatar_id]);
      address.append(QLatin1String("-C103.png"));
      fmt = Avatar::Format::PNG;
    } else {
      address.append(QLatin1String(
          "https://qan.interia.pl/chat/applet/chat_resources/images/avatars/"));

      if (avatarId.length() == 46) {
        address.append(QLatin1String("temporary"));
      } else if (avatarId.length() == 22) {
        address.append(QLatin1String("users"));
      }
      address.append(QLatin1Char('/'));
      address.append(avatarId);
      address.append(QLatin1String(".jpg"));
      fmt = Avatar::Format::JPG;
    }

    auto request = QNetworkRequest(QUrl(address));
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                         QNetworkRequest::PreferCache);
    auto reply = mNAM->get(request);
    auto nickname = user.mLogin;
    QObject::connect(reply, &QNetworkReply::finished, [=]() {
      if (reply->error() == QNetworkReply::NoError) {
        mAvatarCache[avatarId] = Avatar{fmt, reply->readAll()};
        fetchedFn();
      }
      reply->deleteLater();
    });
  }

  bool needsDownload(const User &user) const {
    return !user.mAvatarId.isEmpty() && !mAvatarCache.contains(user.mAvatarId);
  }

  bool hasAvatar(const User &user) const {
    return !user.mAvatarId.isEmpty() && mAvatarCache.contains(user.mAvatarId);
  }

  const Avatar &getAvatar(const User &user) const {
    auto it = mAvatarCache.find(user.mAvatarId);
    Q_ASSERT(it != std::end(mAvatarCache));
    return it.value();
  }

private:
  QNetworkAccessManager *const mNAM;
  QHash<QString, Avatar> mAvatarCache;
};

} // namespace Czateria

#endif // AVATARHANDLER_H
