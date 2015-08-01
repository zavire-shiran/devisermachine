#pragma once

#include <fstream>
#include <ostream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

using std::string;
using std::shared_ptr;
using std::vector;
using std::ostream;

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
    virtual ~lispobj();

    virtual void print(ostream& out = std::cout) = 0;
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
    void add_import(shared_ptr<module> mod);
    // this is honestly kind of gross
    void set_ismodulescope(bool ismodulescope);

    shared_ptr<lispobj> getval(string name);
    shared_ptr<lispobj> getfun(string name);
    const vector< shared_ptr<module> >& get_imports() const;
    shared_ptr<module> find_module(shared_ptr<lispobj> module_prefix);

    void dump();

private:
    std::map<string, shared_ptr<lispobj> > valbindings;
    std::map<string, shared_ptr<lispobj> > funbindings;
    std::shared_ptr<lexicalscope> parent;
    std::vector< shared_ptr<module> > imports;

    // this is honestly kind of gross
    bool ismodulescope;
};

class nil : public lispobj {
public:
    nil();
    virtual void print(ostream& out = std::cout);
};

class symbol : public lispobj {
public:
    symbol(string sn);
    string name() const;
    virtual void print(ostream& out = std::cout);

private:
    string symname;
};

class cons : public lispobj {
public:
    cons(shared_ptr<lispobj> a, shared_ptr<lispobj> d);
    shared_ptr<lispobj> car() const;
    shared_ptr<lispobj> cdr() const;

    void set_car(shared_ptr<lispobj> a);
    void set_cdr(shared_ptr<lispobj> d);

    virtual void print(ostream& out = std::cout);

private:
    shared_ptr<lispobj> first;
    shared_ptr<lispobj> second;
};

class number : public lispobj {
public:
    number(int num);
    int value() const;

    virtual void print(ostream& out = std::cout);

private:
    int num;
};

class lispstring : public lispobj {
public:
    explicit lispstring(const string& str);

    void append(shared_ptr<lispstring> lstr);

    const string& get_contents() const;

    virtual void print(ostream& out = std::cout);
    virtual void repr(ostream& out = std::cout);

private:
    string contents;
};

class lispfunc : public lispobj {
public:
    lispfunc(shared_ptr<lispobj> _args,
             shared_ptr<lexicalscope> _closure,
             shared_ptr<lispobj> _code);

    virtual void print(ostream& out = std::cout);

    shared_ptr<lispobj> args;
    shared_ptr<lexicalscope> closure;
    shared_ptr<lispobj> code;
};

class macro : public lispobj {
public:
    macro(shared_ptr<lispobj> _args,
          shared_ptr<lexicalscope> _closure,
          shared_ptr<lispobj> _code);

    virtual void print(ostream& out = std::cout);

    shared_ptr<lispobj> args;
    shared_ptr<lexicalscope> closure;
    shared_ptr<lispobj> code;
};

class cfunc : public lispobj {
public:
    cfunc(std::function<shared_ptr<lispobj>(vector<shared_ptr<lispobj> >)> f);

    virtual void print(ostream& out = std::cout);

    std::function<shared_ptr<lispobj>(vector<shared_ptr<lispobj> >)> func;
};

class fileinputport : public lispobj {
public:
    fileinputport(string fname);

    shared_ptr<lispobj> read();
    shared_ptr<lispobj> readchar();

    virtual void print(ostream& out = std::cout);

private:
    string filename;
    std::ifstream instream;
};

class module : public lispobj {
public:
    module(shared_ptr<lispobj> _name, shared_ptr<lexicalscope> enc_scope);

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

    virtual void print(ostream& out = std::cout);

private:
    bool ismodulecommand(shared_ptr<lispobj> command);

    shared_ptr<lispobj> name;
    vector< shared_ptr<symbol> > exports;
    vector< shared_ptr<lispobj> > imports;
    shared_ptr<lexicalscope> module_scope;
    vector< shared_ptr<lispobj> > defines;
    vector< shared_ptr<lispobj> > initblocks;
    bool inited;
};

class syntaxlocation {
public:
    syntaxlocation(string name, int linenum, int charnum);

    string name;
    int linenum;
    int charnum;
};

class syntax {
public:
    syntax(shared_ptr<syntaxlocation> loc, shared_ptr<syntax> par);
    virtual ~syntax();

    shared_ptr<syntaxlocation> get_location();
    shared_ptr<syntax> get_parent();
    void set_parent(shared_ptr<syntax> p);

private:
    shared_ptr<syntaxlocation> location;
    shared_ptr<syntax> parent;
};

class syntaxnil : public syntax, public nil {
public:
    syntaxnil(shared_ptr<syntaxlocation> loc, shared_ptr<syntax> par);
};

class syntaxsymbol : public syntax, public symbol {
public:
    syntaxsymbol(string sn, shared_ptr<syntaxlocation> loc, shared_ptr<syntax> par);
};

class syntaxcons : public syntax, public cons {
public:
    syntaxcons(shared_ptr<lispobj> a,
               shared_ptr<lispobj> d,
               shared_ptr<syntaxlocation> loc,
               shared_ptr<syntax> par);
};

class syntaxnumber : public syntax, public number {
public:
    syntaxnumber(int num, shared_ptr<syntaxlocation> loc, shared_ptr<syntax> par);
};

class syntaxstring : public syntax, public lispstring {
public:
    syntaxstring(const string& str, shared_ptr<syntaxlocation> loc, shared_ptr<syntax> par);
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

class reader {
public:
    reader(shared_ptr<std::istream> in, string name);
    shared_ptr<lispobj> read(shared_ptr<syntax> parent = nullptr);
    vector< shared_ptr<lispobj> > readall();

private:
    char get_char();
    char peek_char();

    shared_ptr<std::istream> input;
    string streamname;
    int linenum;
    int colnum;
};

shared_ptr<lispobj> read(string str);
vector< shared_ptr<lispobj> > readall(string str);

shared_ptr<lispobj> eval(shared_ptr<lispobj> code,
                         shared_ptr<lexicalscope> tls);
shared_ptr<module> make_builtins_module(shared_ptr<lexicalscope> top_level_scope);
bool istrue(shared_ptr<lispobj> lobj);
