#include "tst_captcha.h"

#include <gtest/gtest.h>

int main(int argc, char **argv) {
  qInstallMessageHandler([](auto, auto, auto) {});
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
