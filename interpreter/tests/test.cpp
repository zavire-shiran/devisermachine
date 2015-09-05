#include "../deviser.hpp"
#include "gtest/gtest.h"

TEST(DeviserBase, NilEq) {
    shared_ptr<lispobj> n(new nil);

    EXPECT_PRED2(eq, n, n);
}

TEST(DeviserBase, NumEq) {
    shared_ptr<lispobj> num1(new number(1));
    shared_ptr<lispobj> num2(new number(1));
    shared_ptr<lispobj> num3(new number(10));

    EXPECT_PRED2(eq, num1, num1);
    EXPECT_PRED2(eq, num2, num2);
    EXPECT_PRED2(eq, num3, num3);

    EXPECT_FALSE(eq(num1, num2));
    EXPECT_FALSE(eq(num1, num3));
    EXPECT_FALSE(eq(num2, num3));

    EXPECT_FALSE(eq(num2, num1));
    EXPECT_FALSE(eq(num2, num1));
    EXPECT_FALSE(eq(num3, num2));
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

    EXPECT_FALSE(eq(n, num));
    EXPECT_FALSE(eq(num, n));
}

TEST(DeviserBase, NumEqv) {
    shared_ptr<lispobj> zero(new number(0));
    shared_ptr<lispobj> zero2(new number(0));
    shared_ptr<lispobj> one(new number(1));
    shared_ptr<lispobj> one2(new number(1));

    EXPECT_PRED2(eqv, zero, zero);
    EXPECT_PRED2(eqv, zero2, zero2);
    EXPECT_PRED2(eqv, zero, zero2);
    EXPECT_PRED2(eqv, zero2, zero);

    EXPECT_PRED2(eqv, one, one);
    EXPECT_PRED2(eqv, one2, one2);
    EXPECT_PRED2(eqv, one, one2);
    EXPECT_PRED2(eqv, one2, one);

    EXPECT_FALSE(eqv(zero, one));
    EXPECT_FALSE(eqv(one, zero));
}

TEST(DeviserBase, ConsEqv) {
    shared_ptr<lispobj> c1(new cons(std::make_shared<number>(0), std::make_shared<nil>()));
    shared_ptr<lispobj> c2(new cons(std::make_shared<number>(0), std::make_shared<nil>()));

    EXPECT_PRED2(eqv, c1, c1);
    EXPECT_PRED2(eqv, c2, c2);
    EXPECT_FALSE(eqv(c1, c2));
    EXPECT_FALSE(eqv(c2, c1));
}

TEST(DeviserBase, ConsEqual) {
    shared_ptr<lispobj> c1(new cons(std::make_shared<number>(0), std::make_shared<nil>()));
    shared_ptr<lispobj> c2(new cons(std::make_shared<number>(0), std::make_shared<nil>()));
    shared_ptr<lispobj> c3(new cons(std::make_shared<number>(1), std::make_shared<nil>()));

    EXPECT_PRED2(equal, c1, c1);
    EXPECT_PRED2(equal, c2, c2);
    EXPECT_PRED2(equal, c1, c2);
    EXPECT_PRED2(equal, c2, c1);

    EXPECT_FALSE(equal(c1, c3));
    EXPECT_FALSE(equal(c3, c1));
}

TEST(DeviserBase, StringEqual) {
    shared_ptr<lispobj> str1(new lispstring("asdf"));
    shared_ptr<lispobj> str2(new lispstring("asdf"));
    shared_ptr<lispobj> str3(new lispstring("Some other string"));

    EXPECT_PRED2(equal, str1, str1);
    EXPECT_PRED2(equal, str2, str2);
    EXPECT_PRED2(equal, str1, str2);
    EXPECT_PRED2(equal, str2, str1);

    EXPECT_FALSE(equal(str1, str3));
    EXPECT_FALSE(equal(str3, str1));
}

TEST(DeviserBase, ConsSetCar) {
    shared_ptr<lispobj> str1(new lispstring("asdf"));
    shared_ptr<lispobj> str2(new lispstring("qwer"));
    shared_ptr<lispobj> n(new nil());
    shared_ptr<cons> c(new cons(str1, n));

    EXPECT_EQ(str1, c->car());

    c->set_car(str2);

    EXPECT_EQ(str2, c->car());
}

TEST(DeviserBase, ConsSetCdr) {
    shared_ptr<lispobj> str1(new lispstring("asdf"));
    shared_ptr<lispobj> str2(new lispstring("qwer"));
    shared_ptr<lispobj> n(new nil());
    shared_ptr<cons> c(new cons(n, str1));

    EXPECT_EQ(str1, c->cdr());

    c->set_cdr(str2);

    EXPECT_EQ(str2, c->cdr());
}

TEST(DeviserBase, NilPrint) {
    shared_ptr<lispobj> n(new nil());
    std::stringstream ss;

    n->print(ss);

    EXPECT_STREQ("'()", ss.str().c_str());
}

TEST(DeviserBase, SymbolPrint) {
    shared_ptr<lispobj> sym(new symbol("symname"));
    std::stringstream ss;

    sym->print(ss);

    EXPECT_STREQ("symname", ss.str().c_str());
}

TEST(DeviserBase, ProperListPrint) {
    shared_ptr<lispobj> c(new cons(std::make_shared<number>(1),
                                   std::make_shared<cons>(std::make_shared<number>(2),
                                                          std::make_shared<nil>())));
    std::stringstream ss;

    c->print(ss);

    EXPECT_STREQ("(1 2)", ss.str().c_str());
}

TEST(DeviserBase, ImproperListPrint) {
    shared_ptr<lispobj> c(new cons(std::make_shared<number>(1),
                                   std::make_shared<number>(2)));
    std::stringstream ss;

    c->print(ss);

    EXPECT_STREQ("(1 . 2)", ss.str().c_str());
}

TEST(DeviserBase, readNumber) {
    shared_ptr<lispobj> readobj(read("1"));
    shared_ptr<syntaxnumber> num = std::dynamic_pointer_cast<syntaxnumber>(readobj);

    ASSERT_EQ(readobj, num);
    EXPECT_EQ(1, num->value());
    EXPECT_EQ(1, num->get_location()->linenum);
    EXPECT_EQ(1, num->get_location()->charnum);
    EXPECT_EQ(nullptr, num->get_parent());
}

TEST(DeviserBase, readNil) {
    shared_ptr<lispobj> readobj(read("()"));
    shared_ptr<syntaxnil> n = std::dynamic_pointer_cast<syntaxnil>(readobj);

    ASSERT_EQ(readobj, n);
    EXPECT_EQ(1, n->get_location()->linenum);
    EXPECT_EQ(1, n->get_location()->charnum);
    EXPECT_EQ(nullptr, n->get_parent());
}

TEST(DeviserBase, readSymbol) {
    shared_ptr<lispobj> readobj(read("testsymbol"));
    shared_ptr<syntaxsymbol> sym = std::dynamic_pointer_cast<syntaxsymbol>(readobj);

    ASSERT_EQ(readobj, sym);
    EXPECT_STREQ("testsymbol", sym->name().c_str());
    EXPECT_EQ(1, sym->get_location()->linenum);
    EXPECT_EQ(1, sym->get_location()->charnum);
}

TEST(DeviserBase, readCons) {
    shared_ptr<lispobj> readobj(read(" ( 1 ) "));
    shared_ptr<syntaxcons> c = std::dynamic_pointer_cast<syntaxcons>(readobj);

    ASSERT_EQ(readobj, c);
    EXPECT_EQ(1, c->get_location()->linenum);
    EXPECT_EQ(2, c->get_location()->charnum);
    EXPECT_EQ(nullptr, c->get_parent());

    shared_ptr<lispobj> conscar = c->car();
    shared_ptr<lispobj> conscdr = c->cdr();
    shared_ptr<syntaxnumber> num = std::dynamic_pointer_cast<syntaxnumber>(conscar);
    shared_ptr<syntaxnil> n = std::dynamic_pointer_cast<syntaxnil>(conscdr);

    ASSERT_EQ(conscar, num);
    ASSERT_EQ(conscdr, n);
    EXPECT_EQ(1, num->value());
    EXPECT_EQ(1, num->get_location()->linenum);
    EXPECT_EQ(4, num->get_location()->charnum);
    EXPECT_EQ(c, num->get_parent());
    EXPECT_EQ(1, n->get_location()->linenum);
    EXPECT_EQ(6, n->get_location()->charnum);
    EXPECT_EQ(nullptr, n->get_parent());
}

TEST(DeviserBase, readComment) {
    shared_ptr<lispobj> readobj(read(" (;comment\n1)"));
    shared_ptr<syntaxcons> c = std::dynamic_pointer_cast<syntaxcons>(readobj);

    ASSERT_EQ(readobj, c);
    EXPECT_EQ(1, c->get_location()->linenum);
    EXPECT_EQ(2, c->get_location()->charnum);
    EXPECT_EQ(nullptr, c->get_parent());

    shared_ptr<lispobj> conscar = c->car();
    shared_ptr<lispobj> conscdr = c->cdr();
    shared_ptr<syntaxnumber> num = std::dynamic_pointer_cast<syntaxnumber>(conscar);
    shared_ptr<syntaxnil> n = std::dynamic_pointer_cast<syntaxnil>(conscdr);

    ASSERT_EQ(conscar, num);
    ASSERT_EQ(conscdr, n);
    EXPECT_EQ(1, num->value());
    EXPECT_EQ(2, num->get_location()->linenum);
    EXPECT_EQ(1, num->get_location()->charnum);
    EXPECT_EQ(c, num->get_parent());
    EXPECT_EQ(2, n->get_location()->linenum);
    EXPECT_EQ(2, n->get_location()->charnum);
    EXPECT_EQ(nullptr, n->get_parent());
}

TEST(DeviserBase, readComment2) {
    shared_ptr<lispobj> readobj(read("((1) ;comment\n)"));
    shared_ptr<syntaxcons> c = std::dynamic_pointer_cast<syntaxcons>(readobj);

    ASSERT_EQ(readobj, c);
    EXPECT_EQ(nullptr, c->get_parent());

    shared_ptr<lispobj> conscar = c->car();
    shared_ptr<lispobj> conscdr = c->cdr();
    shared_ptr<syntaxcons> c2 = std::dynamic_pointer_cast<syntaxcons>(conscar);
    shared_ptr<syntaxnil> n = std::dynamic_pointer_cast<syntaxnil>(conscdr);

    ASSERT_EQ(conscar, c2);
    ASSERT_EQ(conscdr, n);

    conscar = c2->car();
    conscdr = c2->cdr();
    shared_ptr<syntaxnumber> num = std::dynamic_pointer_cast<syntaxnumber>(conscar);
    n = std::dynamic_pointer_cast<syntaxnil>(conscdr);

    ASSERT_EQ(conscar, num);
    ASSERT_EQ(conscdr, n);
    EXPECT_EQ(1, num->value());
}

TEST(DeviserBase, readString) {
    shared_ptr<lispobj> readobj(read("\"\\a\\s\\d\\f\\nThis is a string.\""));
    shared_ptr<syntaxstring> str = std::dynamic_pointer_cast<syntaxstring>(readobj);

    ASSERT_EQ(readobj, str);
    EXPECT_STREQ("asdf\nThis is a string.", str->get_contents().c_str());
    EXPECT_EQ(nullptr, str->get_parent());
}

TEST(DeviserBase, appendString) {
    shared_ptr<lispstring> str1(new lispstring("asdf"));
    shared_ptr<lispstring> str2(new lispstring("qwer"));

    EXPECT_STREQ("asdf", str1->get_contents().c_str());
    EXPECT_STREQ("qwer", str2->get_contents().c_str());

    str1->append(str2);

    EXPECT_STREQ("asdfqwer", str1->get_contents().c_str());
}

TEST(DeviserBase, printString) {
    shared_ptr<lispobj> str(new lispstring("asdf\n"));
    std::stringstream ss;

    str->print(ss);

    EXPECT_STREQ("asdf\n", ss.str().c_str());
}

TEST(DeviserBase, reprString) {
    shared_ptr<lispstring> str(new lispstring("\"\\\tasdf\n"));
    std::stringstream ss;

    str->repr(ss);

    EXPECT_STREQ("\"\\\"\\\\\\tasdf\\n\"", ss.str().c_str());
}

TEST(DeviserEval, NumberConstant) {
    shared_ptr<lispobj> zero(new number(0));
    shared_ptr<lispobj> one(new number(1));
    shared_ptr<lispobj> two(new number(2));
    shared_ptr<lexicalscope> scope(new lexicalscope);

    EXPECT_EQ(zero, eval(zero, scope));
    EXPECT_EQ(one, eval(one, scope));
    EXPECT_EQ(two, eval(two, scope));
}

TEST(DeviserEval, StringConstant) {
    shared_ptr<lispobj> str(new lispstring("This is a string"));
    shared_ptr<lexicalscope> scope(new lexicalscope);

    EXPECT_EQ(str, eval(str, scope));
}

TEST(DeviserEval, VariableLookup) {
    shared_ptr<lispobj> val(new number(0));
    shared_ptr<lexicalscope> scope(new lexicalscope);
    shared_ptr<symbol> varname(new symbol("testval"));

    scope->defval("testval", val);

    EXPECT_EQ(val, eval(varname, scope));
}

TEST(lexicalscope, getvalUndefined) {
    shared_ptr<lexicalscope> scope(new lexicalscope);

    EXPECT_PRED2(eq, scope->getval("undefinedvariable"), std::make_shared<nil>());
}

TEST(lexicalscope, getvalDefined) {
    shared_ptr<lexicalscope> scope(new lexicalscope);
    shared_ptr<number> zero(new number(0));

    scope->defval("testvar", zero);

    EXPECT_EQ(zero, scope->getval("testvar"));
}

TEST(lexicalscope, getvalFromParent) {
    shared_ptr<lexicalscope> parentscope(new lexicalscope);
    shared_ptr<lexicalscope> childscope(new lexicalscope(parentscope));
    shared_ptr<lispobj> zero (new number(0));

    parentscope->defval("testvar", zero);

    EXPECT_EQ(zero, childscope->getval("testvar"));
}

TEST(lexicalscope, getfunUndefined) {
    shared_ptr<lexicalscope> scope(new lexicalscope);

    EXPECT_PRED2(eq, scope->getfun("undefinedfunction"), std::make_shared<nil>());
}

TEST(lexicalscope, getfunDefined) {
    shared_ptr<lexicalscope> scope(new lexicalscope);
    shared_ptr<lispobj> zero(new number(0));

    scope->defun("testfun", zero);

    EXPECT_EQ(zero, scope->getfun("testfun"));
}

TEST(lexicalscope, getfunFromParent) {
    shared_ptr<lexicalscope> parent(new lexicalscope);
    shared_ptr<lexicalscope> child(new lexicalscope(parent));
    shared_ptr<lispobj> zero(new number(0));

    parent->defun("testfun", zero);

    EXPECT_EQ(zero, child->getfun("testfun"));
}

shared_ptr<lispobj> find_fun_in_module(shared_ptr<module> mod, string name);

TEST(lexicalscope, find_fun_in_moduleFailure) {
    shared_ptr<lexicalscope> scope(new lexicalscope);
    shared_ptr<module> mod(new module(std::make_shared<symbol>("testmod"), scope));

    EXPECT_EQ(nullptr, find_fun_in_module(mod, "undefinedfun"));
}

TEST(lexicalscope, find_fun_in_moduleSuccess) {
    shared_ptr<lexicalscope> scope(new lexicalscope);
    shared_ptr<module> mod(new module(std::make_shared<symbol>("testmod"), scope));

    mod->add_export(std::make_shared<symbol>("testfun"));

    EXPECT_PRED2(eq, std::make_shared<nil>(), find_fun_in_module(mod, "testfun"));
}

TEST(lexicalscope, getvalFromModule) {
    shared_ptr<lexicalscope> scope(new lexicalscope);
    shared_ptr<module> mod(new module(std::make_shared<symbol>("testmod"), scope));
    shared_ptr<lispobj> zero(new number(0));

    scope->add_import(mod);

    mod->defval_and_export("testval", zero);

    EXPECT_EQ(zero, scope->getval("testval"));
}

TEST(lexicalscope, getfunFromModule) {
    shared_ptr<lexicalscope> scope(new lexicalscope);
    shared_ptr<module> mod(new module(std::make_shared<symbol>("testmod"), scope));
    shared_ptr<lispobj> zero(new number(0));

    scope->add_import(mod);

    mod->defun_and_export("testfun", zero);

    EXPECT_EQ(zero, scope->getfun("testfun"));
}
