#include <memory>
#include <string>
#include <iostream>
#include "deviser.hpp"

using std::shared_ptr;
using std::cout;
using std::endl;

int main(int /*argc*/, char** /*argv*/)
{
    cout << "Begin" << endl;
    shared_ptr<nil> nil1(new nil());
    shared_ptr<number> n1(new number(0));
    shared_ptr<number> n2(new number(1));
    shared_ptr<number> n3(new number(1));
    shared_ptr<cons> c1(new cons(n1, n2));
    shared_ptr<cons> c2(new cons(n1, n2));
    shared_ptr<cons> c3(new cons(n2, nil1));
    shared_ptr<cons> l1(new cons(n1, c3));
    cout << "One" << endl;
    auto env = make_standard_env();
    cout << "Two" << endl;
    env->set("a", n2);
    cout << "Three" << endl;

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

    cout << "(print (read \"abc\")) ";
    print(read("abc"));
    cout << endl;

    cout << "(print (read \"(123 abc)\")) ";
    print(read("(123 abc)"));
    cout << endl << endl;

    cout << "(print (eval (read \"123\")))" << endl;
    print(eval(read("123"), env));
    cout << endl;

    cout << "(print (eval (read \"a\"))) => ";
    print(eval(read("a"), env));
    cout << endl;

    cout << "(print (eval (read \"(+ 1 1)\"))) => ";
    print(eval(read("(+ 1 1)"), env));
    cout << endl;

    cout << "(print (eval (read \"(+ a 3)\"))) => ";
    print(eval(read("(+ a 3)"), env));
    cout << endl;
}
