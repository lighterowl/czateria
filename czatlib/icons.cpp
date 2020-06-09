#include "icons.h"

#include <array>

#include <QDebug>
#include <QRegularExpression>

namespace {
struct Icon {
  QString name;
  QString text;
  QString emoji;
};

/* icons are sent from the server as <icon>x</icon> tags, where x is the index
 to the table below. it's the client's responsibility to replace the tag with
the appropriate icon. similarly, when sending messages, the "text icons"
available as the second member of the struct are replaced with an appropriate
<icon>x</icon> tag.
icons are downloaded from
https://qan.interia.pl/chat/applet/chat_resources/images/chat_imgs/icon_%1.gif
where %1 is the icon's name, available as the first member. */

// clang-format doesn't like utf8 chars, and I don't like hex escapes. ergo,
// clang-format needs to take a break here.

// clang-format off
const std::array<Icon, 33> icons = {{
    {QLatin1String("biggrin"), QLatin1String(":]"), QString::fromUtf8("😁")},
    {QLatin1String("biggrin"), QLatin1String(":-]"), QString::fromUtf8("😁")},
    {QLatin1String("smile1"), QLatin1String(":)"), QString::fromUtf8("🙂")},
    {QLatin1String("smile"), QLatin1String(":-)"), QString::fromUtf8("😃")},
    {QLatin1String("frown"), QLatin1String(":("), QString::fromUtf8("☹️")},
    {QLatin1String("frown"), QLatin1String(":-("), QString::fromUtf8("☹️")},
    {QLatin1String("eek"), QLatin1String(":o"), QString::fromUtf8("😱")},
    {QLatin1String("eek1"), QLatin1String(":-o"), QString::fromUtf8("😲")},
    {QLatin1String("confused"), QLatin1String(":-?"), QString::fromUtf8("😕")},
    {QLatin1String("confused"), QLatin1String(":-/"), QString::fromUtf8("😕")},
    {QLatin1String("cool"), QLatin1String("8)"), QString::fromUtf8("😎")},
    {QLatin1String("cool"), QLatin1String("8-)"), QString::fromUtf8("😎")},
    {QLatin1String("lol"), QLatin1String(":-D"), QString::fromUtf8("😄")},
    {QLatin1String("lol"), QLatin1String(":D"), QString::fromUtf8("😄")},
    {QLatin1String("x"), QLatin1String(":x"), QString::fromUtf8("🤐")},
    {QLatin1String("x"), QLatin1String(":-x"), QString::fromUtf8("🤐")},
    {QLatin1String("razz"), QLatin1String(":P"), QString::fromUtf8("😛")},
    {QLatin1String("razz"), QLatin1String(":-P"), QString::fromUtf8("😛")},
    {QLatin1String("cry"), QLatin1String(":'-("), QString::fromUtf8("😢")},
    {QLatin1String("cry"), QLatin1String(":'("), QString::fromUtf8("😢")},
    {QLatin1String("evil"), QLatin1String(">:-["), QString::fromUtf8("😠")},
    {QLatin1String("evil"), QLatin1String(">:["), QString::fromUtf8("😠")},
    {QLatin1String("rolleyes"), QLatin1String(":roll:"), QString::fromUtf8("🙄")},
    {QLatin1String("wink"), QLatin1String(";)"), QString::fromUtf8("😉")},
    {QLatin1String("wink"), QLatin1String(";-)"), QString::fromUtf8("😉")},
    {QLatin1String("cmok"), QLatin1String(":-*"), QString::fromUtf8("😘")},
    {QLatin1String("cmok"), QLatin1String(":*"), QString::fromUtf8("😘")},
    {QLatin1String("angel"), QLatin1String("o:-)"), QString::fromUtf8("😇")},
    {QLatin1String("devil"), QLatin1String("];-)"), QString::fromUtf8("😈")},
    {QLatin1String("cunning"), QLatin1String(":>"), QString::fromUtf8("😏")},
    {QLatin1String("hmmm"), QLatin1String(":-|"), QString::fromUtf8("😐")},
    {QLatin1String("hmmm"), QLatin1String(":|"), QString::fromUtf8("😐")},
    {QLatin1String("we_flower"), QLatin1String(":HIDE:"), QString::fromUtf8("🌼")},
}};
} // namespace

namespace Czateria {
QString convertRawMessage(const QString &str, IconReplaceMode replaceMode) {
  const auto member = replaceMode == IconReplaceMode::Text ? &Icon::text : &Icon::emoji;
  static const auto re =
      QRegularExpression(QLatin1String("<icon>(\\d+)</icon>"),
                         QRegularExpression::OptimizeOnFirstUsageOption);
  auto rv = str;
  auto it = re.globalMatch(str);
  int offset = 0;
  while (it.hasNext()) {
    auto m = it.next();
    auto iconNumber = m.captured(1).toInt();
    if (iconNumber < 0 || iconNumber >= static_cast<int>(icons.size())) {
      continue;
    }
    auto &&icon = icons[static_cast<unsigned>(iconNumber)];
    auto &&replacement = icon.*member;
    rv.replace(m.capturedStart() + offset, m.capturedLength(), replacement);
    offset += (replacement.length() - m.capturedLength());
  }
  // messages tend to contain an embedded \u0000 for some reason. there's no
  // real point in keeping them.
  rv.remove(QChar(0));
  return rv;
}

QString textIconsToTags(const QString &str) {
  auto rv = str;
  for (size_t i = 0; i < icons.size(); ++i) {
    auto &&icon = icons[i];
    const auto replacement = QString(QLatin1String("<icon>%1</icon>")).arg(i);
    rv.replace(icon.text, replacement);
    rv.replace(icon.emoji, replacement);
  }
  return rv;
}
} // namespace Czateria
