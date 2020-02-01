#ifndef TST_BAR_H
#define TST_BAR_H

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

using namespace testing;

TEST(Foo, Bar) {
  EXPECT_EQ(1, 1);
  ASSERT_THAT(0, Eq(0));
}

#endif // TST_BAR_H
