#ifndef CZATLIB_ICONS_H
#define CZATLIB_ICONS_H

#include <QString>

namespace Czateria {

enum class IconReplaceMode { Text, Emoji };

QString convertRawMessage(const QString &str, IconReplaceMode);
QString textIconsToTags(const QString &str);
} // namespace Czateria

#endif
