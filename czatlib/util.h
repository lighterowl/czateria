#ifndef UTIL_H
#define UTIL_H

#include <algorithm>
#include <array>
#include <tuple>

class QValidator;

namespace CzateriaUtil {

template <typename FromType, typename ToType, std::size_t N>
bool convert(FromType in, ToType &out,
             const std::array<std::tuple<FromType, ToType>, N> &mapping) {
  const auto end = std::end(mapping);
  auto it = std::find_if(std::begin(mapping), end,
                         [=](auto &&t) { return std::get<0>(t) == in; });
  if (it != end) {
    out = std::get<1>(*it);
    return true;
  }
  return false;
}

const QValidator *getNicknameValidator();

} // namespace CzateriaUtil

#endif // UTIL_H
