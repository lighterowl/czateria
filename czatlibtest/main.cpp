#include "tst_captcha.h"

#include <gtest/gtest.h>

namespace {
void null_handler(QtMsgType, const QMessageLogContext &, const QString &) {}
} // namespace

int main(int argc, char **argv) {
  qInstallMessageHandler(null_handler);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
