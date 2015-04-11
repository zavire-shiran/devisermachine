#include <memory>
#include <string>
#include <iostream>
#include "deviser.hpp"

using std::shared_ptr;
using std::cout;
using std::endl;

void read_eval_print(const string& lispstr, shared_ptr<environment> env) {
    cout << "(print (eval (read \"" << lispstr << "\"))) => ";
    print(eval(read(lispstr), env));
    cout << endl;
}

int main(int /*argc*/, char** /*argv*/)
{
    shared_ptr<nil> nil1(new nil());
    shared_ptr<number> n1(new number(0));
    shared_ptr<number> n2(new number(1));
    shared_ptr<number> n3(new number(1));
    shared_ptr<cons> c1(new cons(n1, n2));
    shared_ptr<cons> c2(new cons(n1, n2));
    shared_ptr<cons> c3(new cons(n2, nil1));
    shared_ptr<cons> l1(new cons(n1, c3));

    auto env = make_standard_env();
    env->define("a", n2);

    cout << "(eq n2 n3) " << eq(n2, n3) << endl;
    cout << "(eqv n2 n3) " << eqv(n2, n3) << endl;
    cout << "(equal n2 n3) " << equal(n2, n3) << endl;
    cout << endl;
    cout << "(eq c1 c1) " << eq(c1, c1) << endl;
    cout << "(eq c1 c2) " << eq(c1, c2) << endl;
    cout << "(eqv c1 c2) " << eqv(c1, c2) << endl;
    cout << "(equal c1 c2) " << equal(c1, c2) << endl;
    cout << endl;
    cout << "(eq c1 n1) " << eq(c1, n1) << endl;
    cout << "(eqv c1 n1) " << eqv(c1, n1) << endl;
    cout << "(equal c1 n1) " << equal(c1, n1) << endl;
    cout << endl;

    cout << "(print nil1) ";
    print(nil1);
    cout << endl;

    cout << "(print n1) ";
    print(n1);
    cout << endl;

    cout << "(print n2) ";
    print(n2);
    cout << endl;

    cout << "(print c1) ";
    print(c1);
    cout << endl;

    cout << "(print 11) ";
    print(l1);
    cout << endl << endl;

    cout << "(print (read \"123\")) ";
    print(read("123"));
    cout << endl;

    cout << "(print (read \"nil\")) ";
    print(read("nil"));
    cout << endl;

    cout << "(print (read \"abc\")) ";
    print(read("abc"));
    cout << endl;

    cout << "(print (read \"(if nil 0 1)\")) ";
    print(read("(if nil 0 1)"));
    cout << endl;

    cout << "(print (read \"(123 abc )\")) ";
    print(read("(123 abc )"));
    cout << endl << endl;

    read_eval_print("123", env);
    read_eval_print("a", env);
    read_eval_print("( + 1 1 )", env);
    read_eval_print("( + 1 (+ 1 1) )", env);
    read_eval_print("(+ a 3)", env);
    read_eval_print("(- a)", env);
    read_eval_print("(- a 3)", env);
    read_eval_print("(* 10 12)", env);
    read_eval_print("(* a 3)", env);
    read_eval_print("(/ 12 2)", env);
    read_eval_print("(/ 3 a)", env);
    read_eval_print("(/ 12 5)", env);
    read_eval_print("(/ a 3)", env);
    cout << endl;

    shared_ptr<lispfunc> lfunc(new lispfunc(read("(x)"), env, read("((+ x x))")));
    env->define("double", lfunc);
    read_eval_print("(double 1)", env);
    read_eval_print("(double 2)", env);
    read_eval_print("(double 3)", env);
    read_eval_print("(double 4)", env);
    read_eval_print("(double (double 1))", env);
    read_eval_print("(double (double 2))", env);
    read_eval_print("(double (double 3))", env);
    read_eval_print("(double (double 4))", env);

    shared_ptr<lispfunc> lfunc2(new lispfunc(read("(x y)"), env, read("((+ (double x) (double y)))")));
    env->define("double+double", lfunc2);
    read_eval_print("(double+double a 1)", env);
    read_eval_print("(double+double a 4)", env);
    read_eval_print("(double+double 2 3)", env);
    read_eval_print("(double+double 2 5)", env);
    cout << endl;

    read_eval_print("(if nil 0 1)", env);
    read_eval_print("(if 1 0 1)", env);
    read_eval_print("(if nil 0 a)", env);
    read_eval_print("(if 1 0 a)", env);
    read_eval_print("(if nil 0)", env);
    read_eval_print("(if 1 0)", env);

    read_eval_print("((lambda () 1))", env);
    read_eval_print("((lambda (x) x) 0)", env);
    read_eval_print("((lambda (x) x) nil)", env);
    read_eval_print("((lambda (x y) (+ x y)) 1 2)", env);
    read_eval_print("(module (test asdf) (define a 1) (import (test one)) (export a) (init (+ 1 1)))", env);
}
