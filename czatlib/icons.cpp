#include "icons.h"

#include <QRegularExpression>
#include <QDebug>

namespace {
struct Icon {
  QString name;
  QString text;
};

/* icons are sent from the server as <icon>x</icon> tags, where x is the index
 to the table below. it's the client's responsibility to replace the tag with
the appropriate icon. similarly, when sending messages, the "text icons"
available as the second member of the struct are replaced with an appropriate
<icon>x</icon> tag.
icons are downloaded from
https://qan.interia.pl/chat/applet/chat_resources/images/chat_imgs/icon_%1.gif
where %1 is the icon's name, available as the first member. */

const std::array<Icon, 33> icons = {{
    {QLatin1String("biggrin"), QLatin1String(":]")},
    {QLatin1String("biggrin"), QLatin1String(":-]")},
    {QLatin1String("smile1"), QLatin1String(":)")},
    {QLatin1String("smile"), QLatin1String(":-)")},
    {QLatin1String("frown"), QLatin1String(":(")},
    {QLatin1String("frown"), QLatin1String(":-(")},
    {QLatin1String("eek"), QLatin1String(":o")},
    {QLatin1String("eek1"), QLatin1String(":-o")},
    {QLatin1String("confused"), QLatin1String(":-?")},
    {QLatin1String("confused"), QLatin1String(":-/")},
    {QLatin1String("cool"), QLatin1String("8)")},
    {QLatin1String("cool"), QLatin1String("8-)")},
    {QLatin1String("lol"), QLatin1String(":-D")},
    {QLatin1String("lol"), QLatin1String(":D")},
    {QLatin1String("x"), QLatin1String(":x")},
    {QLatin1String("x"), QLatin1String(":-x")},
    {QLatin1String("razz"), QLatin1String(":P")},
    {QLatin1String("razz"), QLatin1String(":-P")},
    {QLatin1String("cry"), QLatin1String(":'-(")},
    {QLatin1String("cry"), QLatin1String(":'(")},
    {QLatin1String("evil"), QLatin1String(">:-[")},
    {QLatin1String("evil"), QLatin1String(">:[")},
    {QLatin1String("rolleyes"), QLatin1String(":roll:")},
    {QLatin1String("wink"), QLatin1String(";)")},
    {QLatin1String("wink"), QLatin1String(";-)")},
    {QLatin1String("cmok"), QLatin1String(":-*")},
    {QLatin1String("cmok"), QLatin1String(":*")},
    {QLatin1String("angel"), QLatin1String("o:-)")},
    {QLatin1String("devil"), QLatin1String("];-)")},
    {QLatin1String("cunning"), QLatin1String(":>")},
    {QLatin1String("hmmm"), QLatin1String(":-|")},
    {QLatin1String("hmmm"), QLatin1String(":|")},
    {QLatin1String("we_flower"), QLatin1String(":HIDE:")},
}};
} // namespace

namespace Czateria {
QString tagsToTextIcons(const QString &str) {
  static const auto re =
      QRegularExpression(QLatin1String("<icon>(\\d+)</icon>")
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
                             ,
                         QRegularExpression::OptimizeOnFirstUsageOption
#endif
      );
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
    rv.replace(m.capturedStart() + offset, m.capturedLength(), icon.text);
    offset += (icon.text.length() - m.capturedLength());
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
    rv.replace(icon.text, QString(QLatin1String("<icon>%1</icon>")).arg(i));
  }
  return rv;
}
} // namespace Czateria
