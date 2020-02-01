#include "util.h"

#include <QRegExpValidator>

namespace CzateriaUtil {
const QValidator *getNicknameValidator() {
  static const QRegExpValidator validator(QRegExp(QLatin1String("[^'@\\$]+")));
  return &validator;
}
} // namespace CzateriaUtil
