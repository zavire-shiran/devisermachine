#include <map>
#include <memory>
#include <string>
#include <vector>

using std::string;
using std::shared_ptr;
using std::vector;

const int INVALID_TYPE = 0;
const int NIL_TYPE = 1;
const int CONS_TYPE = 2;
const int SYMBOL_TYPE = 3;
const int NUMBER_TYPE = 4;
const int FUNC_TYPE = 5;
const int CFUNC_TYPE = 6;

class lispobj {
public:
    lispobj();
    virtual int objtype() const = 0;
};

class environment {
public:
    environment();
    environment(shared_ptr<environment> env);
    void define(string name, shared_ptr<lispobj> value);
    void set(string name, shared_ptr<lispobj> value);
    shared_ptr<lispobj> get(string name);
    void dump();

private:
    std::map<string, shared_ptr<lispobj> > bindings;
    shared_ptr<environment> parent;
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

class lispfunc : public lispobj {
public:
    lispfunc(shared_ptr<lispobj> _args,
             shared_ptr<environment> _closure,
             shared_ptr<lispobj> _code);
    virtual int objtype() const;

    shared_ptr<lispobj> args;
    shared_ptr<environment> closure;
    shared_ptr<lispobj> code;
};

class cfunc : public lispobj {
public:
    cfunc(std::function<shared_ptr<lispobj>(vector<shared_ptr<lispobj> >)> f);
    virtual int objtype() const;

    std::function<shared_ptr<lispobj>(vector<shared_ptr<lispobj> >)> func;
};

bool eq(shared_ptr<lispobj> left, shared_ptr<lispobj> right);
bool eqv(shared_ptr<lispobj> left, shared_ptr<lispobj> right);
bool equal(shared_ptr<lispobj> left, shared_ptr<lispobj> right);

shared_ptr<lispobj> make_list(const vector<shared_ptr<lispobj> >& list_values);

void print(shared_ptr<lispobj> obj);
shared_ptr<lispobj> read(string str);

shared_ptr<lispobj> eval(shared_ptr<lispobj> code, shared_ptr<environment> tle);
shared_ptr<environment> make_standard_env();
