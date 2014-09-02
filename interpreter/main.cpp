#include <memory>
#include <string>
#include <iostream>
#include "deviser.hpp"

using std::shared_ptr;
using std::cout;
using std::endl;

int main(int argc, char** argv)
{
    shared_ptr<number> n1(new number(0));
    shared_ptr<number> n2(new number(1));
    shared_ptr<number> n3(new number(1));
    shared_ptr<cons> c1(new cons(n1, n2));
    shared_ptr<cons> c2(new cons(n1, n2));

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
}
