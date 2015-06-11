#pragma once

#include <fstream>
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
const int STRING_TYPE = 8;
const int FILEINPUTPORT_TYPE = 9;

class lispobj {
public:
    lispobj();

    virtual void print() = 0;
};

class module;

class lexicalscope {
public:
    lexicalscope();
    lexicalscope(shared_ptr<lexicalscope> p);
    void defval(string name, shared_ptr<lispobj> value);
    void defun(string name, shared_ptr<lispobj> value);
    void undefval(string name);
    void undefun(string name);
    void setval(string name, shared_ptr<lispobj> value);
    void setfun(string name, shared_ptr<lispobj> value);
    shared_ptr<lispobj> getval(string name);
    shared_ptr<lispobj> getfun(string name);
    void add_import(shared_ptr<module> mod);
    const vector< shared_ptr<module> >& get_imports() const;
    void dump();

private:
    std::map<string, shared_ptr<lispobj> > valbindings;
    std::map<string, shared_ptr<lispobj> > funbindings;
    std::shared_ptr<lexicalscope> parent;
    std::vector< shared_ptr<module> > imports;
};

class nil : public lispobj {
public:
    nil();
    virtual void print();
};

class symbol : public lispobj {
public:
    symbol(string sn);
    string name() const;
    virtual void print();

private:
    string symname;
};

class cons : public lispobj {
public:
    cons(shared_ptr<lispobj> a, shared_ptr<lispobj> d);
    shared_ptr<lispobj> car() const;
    shared_ptr<lispobj> cdr() const;

    virtual void print();

private:
    shared_ptr<lispobj> first;
    shared_ptr<lispobj> second;
};

class number : public lispobj {
public:
    number(int num);
    int value() const;

    virtual void print();

private:
    int num;
};

class lispstring : public lispobj {
public:
    lispstring();
    explicit lispstring(const string& str);

    void append(shared_ptr<lispstring> lstr);

    const string& get_contents() const;

    virtual void print();

private:
    string contents;
};

class lispfunc : public lispobj {
public:
    lispfunc(shared_ptr<lispobj> _args,
             shared_ptr<lexicalscope> _closure,
             shared_ptr<lispobj> _code);

    virtual void print();

    shared_ptr<lispobj> args;
    shared_ptr<lexicalscope> closure;
    shared_ptr<lispobj> code;
};

class macro : public lispobj {
public:
    macro(shared_ptr<lispobj> _args,
          shared_ptr<lexicalscope> _closure,
          shared_ptr<lispobj> _code);

    virtual void print();

    shared_ptr<lispobj> args;
    shared_ptr<lexicalscope> closure;
    shared_ptr<lispobj> code;
};

class cfunc : public lispobj {
public:
    cfunc(std::function<shared_ptr<lispobj>(vector<shared_ptr<lispobj> >)> f);

    virtual void print();

    std::function<shared_ptr<lispobj>(vector<shared_ptr<lispobj> >)> func;
};

class fileinputport : public lispobj {
public:
    fileinputport(string fname);

    shared_ptr<lispobj> read();
    shared_ptr<lispobj> readchar();

    virtual void print();

private:
    string filename;
    std::ifstream instream;
};

class module : public lispobj {
public:
    module(shared_ptr<lispobj> _name);

    shared_ptr<lispobj> eval(shared_ptr<lispobj> command);

    void add_import(shared_ptr<module> mod);
    void add_export(shared_ptr<symbol> sym);
    void defval(string name, shared_ptr<lispobj> value);
    void defun(string name, shared_ptr<lispobj> value);
    void defval_and_export(string symname, shared_ptr<lispobj> value);
    void defun_and_export(string symname, shared_ptr<lispobj> value);
    void add_init(shared_ptr<lispobj> initblock);

    shared_ptr<lispobj> get_name() const;
    void set_name(shared_ptr<lispobj> newname);
    shared_ptr<lexicalscope> get_bindings() const;
    bool init();

    const vector< shared_ptr<lispobj> >& get_imports() const;
    const vector< shared_ptr<symbol> >& get_exports() const;
    const vector< shared_ptr<lispobj> >& get_initblocks() const;

    virtual void print();

private:
    bool ismodulecommand(shared_ptr<lispobj> command);

    shared_ptr<lispobj> name;
    vector< shared_ptr<symbol> > exports;
    vector< shared_ptr<lispobj> > imports;
    shared_ptr<lexicalscope> scope;
    vector< shared_ptr<lispobj> > defines;
    vector< shared_ptr<lispobj> > initblocks;
    bool inited;
};

bool eq(shared_ptr<lispobj> left, shared_ptr<lispobj> right);
bool eqv(shared_ptr<lispobj> left, shared_ptr<lispobj> right);
bool equal(shared_ptr<lispobj> left, shared_ptr<lispobj> right);

template<typename input_iterator>
shared_ptr<lispobj> make_reverse_list(input_iterator begin,
                                      input_iterator end) {
    shared_ptr<lispobj> ret(new nil());

    for(auto it = begin; it != end; ++it) {
        ret.reset(new cons(*it, ret));
    }

    return ret;
}

void printall(vector< shared_ptr<lispobj> > objs);
shared_ptr<lispobj> read(string str);
vector< shared_ptr<lispobj> > readall(string str);

shared_ptr<lispobj> eval(shared_ptr<lispobj> code,
                         shared_ptr<lexicalscope> tls);
shared_ptr<module> make_builtins_module();
bool istrue(shared_ptr<lispobj> lobj);
