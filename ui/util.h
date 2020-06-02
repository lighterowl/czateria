#ifndef UI_UTIL_H
#define UI_UTIL_H

#include <QMetaObject>
#include <QObject>
#include <memory>

template <typename QObj, typename Signal, typename Slot>
inline void oneshotConnect(QObj *obj, Signal sig, Slot &&func) {
  auto c = std::make_unique<QMetaObject::Connection>();
  auto p = c.get();
  *p = QObject::connect(obj, sig, [uc = std::move(c), func](auto... params) {
    QObject::disconnect(*uc);
    func(params...);
  });
}

#endif // UI_UTIL_H
