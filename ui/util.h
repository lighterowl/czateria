#ifndef UI_UTIL_H
#define UI_UTIL_H

#include <QObject>

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#define QOBJECT_CONNECT_MOVES_FUNCTORS
#include <memory>
#else
#include <QSharedPointer>
#endif

template <typename QObj, typename Signal, typename Slot>
inline void oneshotConnect(QObj *obj, Signal sig, Slot &&func) {
#ifdef QOBJECT_CONNECT_MOVES_FUNCTORS
  auto c = std::make_unique<QMetaObject::Connection>();
  auto p = c.get();
  *p = QObject::connect(obj, sig, [uc = std::move(c), func](auto... params) {
    QObject::disconnect(*uc);
    func(params...);
  });
#else
  auto c = QSharedPointer<QMetaObject::Connection>::create();
  *c = QObject::connect(obj, sig, [c, func](auto... params) {
    QObject::disconnect(*c);
    func(params...);
  });
#endif
}

#endif // UI_UTIL_H
