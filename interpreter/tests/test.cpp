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

TEST(DeviserEval, NumberConstantTest) {
    shared_ptr<lispobj> zero(new number(0));
    shared_ptr<lispobj> one(new number(1));
    shared_ptr<lispobj> two(new number(2));
    shared_ptr<lexicalscope> scope(new lexicalscope);

    ASSERT_EQ(zero, eval(zero, scope));
    ASSERT_EQ(one, eval(one, scope));
    ASSERT_EQ(two, eval(two, scope));
}

TEST(lexicalscope, getvalUndefined) {
    shared_ptr<lexicalscope> scope(new lexicalscope);

    ASSERT_TRUE(eq(scope->getval("undefinedvariable"), std::make_shared<nil>()));
}

TEST(lexicalscope, getvalDefined) {
    shared_ptr<lexicalscope> scope(new lexicalscope);
    shared_ptr<number> zero(new number(0));

    scope->defval("testvar", zero);

    ASSERT_EQ(zero, scope->getval("testvar"));
}

TEST(lexicalscope, getvalFromParent) {
    shared_ptr<lexicalscope> parentscope(new lexicalscope);
    shared_ptr<lexicalscope> childscope(new lexicalscope(parentscope));
    shared_ptr<lispobj> zero (new number(0));

    parentscope->defval("testvar", zero);

    ASSERT_EQ(zero, childscope->getval("testvar"));
}

TEST(lexicalscope, getfunUndefined) {
    shared_ptr<lexicalscope> scope(new lexicalscope);

    ASSERT_TRUE(eq(scope->getfun("undefinedfunction"), std::make_shared<nil>()));
}

TEST(lexicalscope, getfunDefined) {
    shared_ptr<lexicalscope> scope(new lexicalscope);
    shared_ptr<lispobj> zero(new number(0));

    scope->defun("testfun", zero);

    ASSERT_EQ(zero, scope->getfun("testfun"));
}

TEST(lexicalscope, getfunFromParent) {
    shared_ptr<lexicalscope> parent(new lexicalscope);
    shared_ptr<lexicalscope> child(new lexicalscope(parent));
    shared_ptr<lispobj> zero(new number(0));

    parent->defun("testfun", zero);

    ASSERT_EQ(zero, child->getfun("testfun"));
}

shared_ptr<lispobj> find_fun_in_module(shared_ptr<module> mod, string name);

TEST(lexicalscope, find_fun_in_moduleFailure) {
    shared_ptr<lexicalscope> scope(new lexicalscope);
    shared_ptr<module> mod(new module(std::make_shared<symbol>("testmod"), scope));

    ASSERT_EQ(nullptr, find_fun_in_module(mod, "undefinedfun"));
}

TEST(lexicalscope, find_fun_in_moduleSuccess) {
    shared_ptr<lexicalscope> scope(new lexicalscope);
    shared_ptr<module> mod(new module(std::make_shared<symbol>("testmod"), scope));

    mod->add_export(std::make_shared<symbol>("testfun"));

    ASSERT_TRUE(eq(std::make_shared<nil>(), find_fun_in_module(mod, "testfun")));
}
