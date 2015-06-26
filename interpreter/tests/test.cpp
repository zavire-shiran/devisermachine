#include "../deviser.hpp"
#include "gtest/gtest.h"

TEST(Testing, NilEqTest) {
    shared_ptr<lispobj> n(new nil);

    ASSERT_TRUE(eq(n, n));
}

TEST(Testing, NumEqTest) {
    shared_ptr<lispobj> num1(new number(1));
    shared_ptr<lispobj> num2(new number(1));
    shared_ptr<lispobj> num3(new number(10));

    ASSERT_TRUE(eq(num1, num1));
    ASSERT_TRUE(eq(num2, num2));
    ASSERT_TRUE(eq(num3, num3));

    ASSERT_FALSE(eq(num1, num2));
    ASSERT_FALSE(eq(num1, num3));
    ASSERT_FALSE(eq(num2, num3));

    ASSERT_FALSE(eq(num2, num1));
    ASSERT_FALSE(eq(num2, num1));
    ASSERT_FALSE(eq(num3, num2));
}

TEST(Testing, NilNumEqTest) {
    shared_ptr<lispobj> n(new nil);
    shared_ptr<lispobj> num(new number(0));

    ASSERT_FALSE(eq(n, num));
    ASSERT_FALSE(eq(num, n));
}
