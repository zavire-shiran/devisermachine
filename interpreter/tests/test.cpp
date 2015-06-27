#include "../deviser.hpp"
#include "gtest/gtest.h"

TEST(DeviserBase, NilEqTest) {
    shared_ptr<lispobj> n(new nil);

    ASSERT_TRUE(eq(n, n));
}

TEST(DeviserBase, NumEqTest) {
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

TEST(DeviserBase, NilNumEqTest) {
    shared_ptr<lispobj> n(new nil);
    shared_ptr<lispobj> num(new number(0));

    ASSERT_FALSE(eq(n, num));
    ASSERT_FALSE(eq(num, n));
}

TEST(DeviserBase, NumEqvTest) {
    shared_ptr<lispobj> zero(new number(0));
    shared_ptr<lispobj> zero2(new number(0));
    shared_ptr<lispobj> one(new number(1));
    shared_ptr<lispobj> one2(new number(1));

    ASSERT_TRUE(eqv(zero, zero));
    ASSERT_TRUE(eqv(zero2, zero2));
    ASSERT_TRUE(eqv(zero, zero2));
    ASSERT_TRUE(eqv(zero2, zero));

    ASSERT_TRUE(eqv(one, one));
    ASSERT_TRUE(eqv(one2, one2));
    ASSERT_TRUE(eqv(one, one2));
    ASSERT_TRUE(eqv(one2, one));

    ASSERT_FALSE(eqv(zero, one));
    ASSERT_FALSE(eqv(one, zero));
}

TEST(DeviserEval, NumberTest) {
    shared_ptr<lispobj> zero(new number(0));
    shared_ptr<lispobj> one(new number(1));
    shared_ptr<lispobj> two(new number(2));
    shared_ptr<lexicalscope> scope(new lexicalscope);

    ASSERT_EQ(eval(zero, scope), zero);
    ASSERT_EQ(eval(one, scope), one);
    ASSERT_EQ(eval(two, scope), two);
}

TEST(lexicalscope, get_valUndefined) {
    shared_ptr<lexicalscope> scope(new lexicalscope);

    ASSERT_TRUE(eq(scope->getval("undefinevariable"), std::make_shared<nil>()));
}

TEST(lexicalscope, get_valDefined) {
    shared_ptr<lexicalscope> scope(new lexicalscope);
    shared_ptr<number> zero(new number(0));

    scope->defval("testvar", zero);

    ASSERT_EQ(scope->getval("testvar"), zero);
}

TEST(lexicalscope, get_valFromParent) {
    shared_ptr<lexicalscope> parentscope(new lexicalscope);
    shared_ptr<lexicalscope> childscope(new lexicalscope(parentscope));
    shared_ptr<lispobj> zero (new number(0));

    parentscope->defval("testvar", zero);

    ASSERT_EQ(zero, childscope->getval("testvar"));
}
