#include <memory>
#include <string>

using std::string;
using std::shared_ptr;

const int INVALID_TYPE = 0;
const int NIL_TYPE = 1;
const int CONS_TYPE = 2;
const int SYMBOL_TYPE = 3;
const int NUMBER_TYPE = 4;

class lispobj {
public:
    lispobj();
    virtual int objtype() const = 0;
};

class nil : public lispobj {
public:
    nil();
    virtual int objtype() const;
};

class symbol : public lispobj {
public:
    symbol(string sn);
    virtual int objtype() const;
    string name() const;

private:
    string symname;
};

class cons : public lispobj {
public:
    cons(shared_ptr<lispobj> a, shared_ptr<lispobj> d);
    virtual int objtype() const;
    shared_ptr<lispobj> car() const;
    shared_ptr<lispobj> cdr() const;

private:
    shared_ptr<lispobj> first;
    shared_ptr<lispobj> second;
};

class number : public lispobj {
public:
    number(int num);
    virtual int objtype() const;
    int value() const;

private:
    int num;
};

bool eq(shared_ptr<lispobj> left, shared_ptr<lispobj> right);
bool eqv(shared_ptr<lispobj> left, shared_ptr<lispobj> right);
bool equal(shared_ptr<lispobj> left, shared_ptr<lispobj> right);

void print(shared_ptr<lispobj> obj);
