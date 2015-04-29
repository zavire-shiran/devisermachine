#pragma once

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
const int MODULE_TYPE = 7;

class lispobj {
public:
    lispobj();
    virtual int objtype() const = 0;
};

class module;
class lexicalscope;

class environment {
public:
    environment();
    shared_ptr<lexicalscope> get_scope();
    void set_scope(shared_ptr<lexicalscope> s);
    void add_module_def(shared_ptr<module> m);
    shared_ptr<module> get_module_by_prefix(shared_ptr<lispobj> prefix);

private:
    shared_ptr<lexicalscope> scope;
    vector< shared_ptr<module> > modules;
};

class lexicalscope {
public:
    lexicalscope(shared_ptr<environment> e);
    lexicalscope(shared_ptr<environment> e, shared_ptr<lexicalscope> p);
    void define(string name, shared_ptr<lispobj> value);
    void set(string name, shared_ptr<lispobj> value);
    shared_ptr<lispobj> get(string name);
    void add_import(shared_ptr<module> mod);
    const vector< shared_ptr<module> >& get_imports() const;
    void dump();

private:
    std::map<string, shared_ptr<lispobj> > bindings;
    std::shared_ptr<lexicalscope> parent;
    std::vector< shared_ptr<module> > imports;
    std::weak_ptr<environment> env;
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
             shared_ptr<lexicalscope> _closure,
             shared_ptr<lispobj> _code);
    virtual int objtype() const;

    shared_ptr<lispobj> args;
    shared_ptr<lexicalscope> closure;
    shared_ptr<lispobj> code;
};

class cfunc : public lispobj {
public:
    cfunc(std::function<shared_ptr<lispobj>(vector<shared_ptr<lispobj> >)> f);
    virtual int objtype() const;

    std::function<shared_ptr<lispobj>(vector<shared_ptr<lispobj> >)> func;
};

class module : public lispobj {
public:
    module(shared_ptr<lispobj> _name, shared_ptr<environment> env);
    void add_import(shared_ptr<lispobj> modname);
    void add_export(shared_ptr<symbol> sym);
    void define(string name, shared_ptr<lispobj> value);
    void add_init(shared_ptr<lispobj> initblock);
    shared_ptr<lispobj> get_name() const;
    shared_ptr<lexicalscope> get_bindings() const;
    bool init(shared_ptr<environment> env);

    const vector< shared_ptr<lispobj> >& get_imports() const;
    const vector< shared_ptr<symbol> >& get_exports() const;
    const vector< shared_ptr<lispobj> >& get_initblocks() const;

    virtual int objtype() const;

private:
    // we actually want a map from symbol to module that we get it from
    // but that's hard because of import all...
    shared_ptr<lispobj> name;
    vector< shared_ptr<symbol> > exports;
    vector< shared_ptr<lispobj> > imports;
    shared_ptr<lexicalscope> scope;
    vector< shared_ptr<lispobj> > initblocks;
    bool inited;
};

bool eq(shared_ptr<lispobj> left, shared_ptr<lispobj> right);
bool eqv(shared_ptr<lispobj> left, shared_ptr<lispobj> right);
bool equal(shared_ptr<lispobj> left, shared_ptr<lispobj> right);

shared_ptr<lispobj> make_list(const vector<shared_ptr<lispobj> >& list_values);

void print(shared_ptr<lispobj> obj);
shared_ptr<lispobj> read(string str);
vector< shared_ptr<lispobj> > readall(string str);

shared_ptr<lispobj> eval(shared_ptr<lispobj> code,
                         shared_ptr<lexicalscope> tls,
                         shared_ptr<environment> env);
shared_ptr<environment> make_standard_env();
