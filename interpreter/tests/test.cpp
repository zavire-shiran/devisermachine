#include "../deviser.hpp"
#include "gtest/gtest.h"

TEST(DeviserBase, NilEq) {
    shared_ptr<lispobj> n(new nil);

    ASSERT_TRUE(eq(n, n));
}

TEST(DeviserBase, NumEq) {
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

TEST(DeviserBase, SymbolEq) {
    shared_ptr<lispobj> sym(new symbol("sym1"));
    shared_ptr<lispobj> sym1(new symbol("sym1"));
    shared_ptr<lispobj> sym2(new symbol("sym2"));

    EXPECT_PRED2(eq, sym, sym);
    EXPECT_PRED2(eq, sym1, sym1);
    EXPECT_PRED2(eq, sym2, sym2);

    EXPECT_PRED2(eq, sym, sym1);
    EXPECT_PRED2(eq, sym1, sym);
    EXPECT_FALSE(eq(sym, sym2));
    EXPECT_FALSE(eq(sym2, sym));
}

TEST(DeviserBase, ConsEq) {
    shared_ptr<lispobj> c1(new cons(std::make_shared<symbol>("a"), std::make_shared<nil>()));
    shared_ptr<lispobj> c2(new cons(std::make_shared<symbol>("a"), std::make_shared<nil>()));

    EXPECT_PRED2(eq, c1, c1);
    EXPECT_PRED2(eq, c2, c2);

    EXPECT_FALSE(eq(c1, c2));
    EXPECT_FALSE(eq(c2, c1));
}

TEST(DeviserBase, NilNumEq) {
    shared_ptr<lispobj> n(new nil);
    shared_ptr<lispobj> num(new number(0));

    ASSERT_FALSE(eq(n, num));
    ASSERT_FALSE(eq(num, n));
}

TEST(DeviserBase, NumEqv) {
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

TEST(DeviserBase, ConsEqv) {
    shared_ptr<lispobj> c1(new cons(std::make_shared<number>(0), std::make_shared<nil>()));
    shared_ptr<lispobj> c2(new cons(std::make_shared<number>(0), std::make_shared<nil>()));

    EXPECT_PRED2(eqv, c1, c1);
    EXPECT_PRED2(eqv, c2, c2);
    EXPECT_FALSE(eqv(c1, c2));
    EXPECT_FALSE(eqv(c2, c1));
}

TEST(DeviserBase, readNumber) {
    shared_ptr<lispobj> readobj(read("1"));
    shared_ptr<number> num = std::dynamic_pointer_cast<number>(readobj);

    ASSERT_EQ(readobj, num);
    EXPECT_EQ(1, num->value());
}

TEST(DeviserBase, readNil) {
    shared_ptr<lispobj> readobj(read("()"));
    shared_ptr<nil> n = std::dynamic_pointer_cast<nil>(readobj);

    ASSERT_EQ(readobj, n);
}

TEST(DeviserBase, readSymbol) {
    shared_ptr<lispobj> readobj(read("testsymbol"));
    shared_ptr<symbol> sym = std::dynamic_pointer_cast<symbol>(readobj);

    ASSERT_EQ(readobj, sym);
    EXPECT_STREQ("testsymbol", sym->name().c_str());
}

TEST(DeviserBase, readCons) {
    shared_ptr<lispobj> readobj(read(" ( 1 ) "));
    shared_ptr<cons> c = std::dynamic_pointer_cast<cons>(readobj);

    ASSERT_EQ(readobj, c);

    shared_ptr<lispobj> conscar = c->car();
    shared_ptr<lispobj> conscdr = c->cdr();
    shared_ptr<number> num = std::dynamic_pointer_cast<number>(conscar);
    shared_ptr<nil> n = std::dynamic_pointer_cast<nil>(conscdr);

    ASSERT_EQ(conscar, num);
    ASSERT_EQ(conscdr, n);
    EXPECT_EQ(1, num->value());
}

TEST(DeviserEval, NumberConstant) {
    shared_ptr<lispobj> zero(new number(0));
    shared_ptr<lispobj> one(new number(1));
    shared_ptr<lispobj> two(new number(2));
    shared_ptr<lexicalscope> scope(new lexicalscope);

    ASSERT_EQ(zero, eval(zero, scope));
    ASSERT_EQ(one, eval(one, scope));
    ASSERT_EQ(two, eval(two, scope));
}

TEST(DeviserEval, StringConstant) {
    shared_ptr<lispobj> str(new lispstring("This is a string"));
    shared_ptr<lexicalscope> scope(new lexicalscope);

    ASSERT_EQ(str, eval(str, scope));
}

TEST(DeviserEval, VariableLookup) {
    shared_ptr<lispobj> val(new number(0));
    shared_ptr<lexicalscope> scope(new lexicalscope);
    shared_ptr<symbol> varname(new symbol("testval"));

    scope->defval("testval", val);

    ASSERT_EQ(val, eval(varname, scope));
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

TEST(lexicalscope, getvalFromModule) {
    shared_ptr<lexicalscope> scope(new lexicalscope);
    shared_ptr<module> mod(new module(std::make_shared<symbol>("testmod"), scope));
    shared_ptr<lispobj> zero(new number(0));

    scope->add_import(mod);

    mod->defval_and_export("testval", zero);

    ASSERT_EQ(zero, scope->getval("testval"));
}

TEST(lexicalscope, getfunFromModule) {
    shared_ptr<lexicalscope> scope(new lexicalscope);
    shared_ptr<module> mod(new module(std::make_shared<symbol>("testmod"), scope));
    shared_ptr<lispobj> zero(new number(0));

    scope->add_import(mod);

    mod->defun_and_export("testfun", zero);

    ASSERT_EQ(zero, scope->getfun("testfun"));
}
