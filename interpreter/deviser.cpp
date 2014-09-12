#include <iostream>
#include <stack>
#include "deviser.hpp"

using std::cout;
using std::endl;

lispobj::lispobj() {}

nil::nil() {}

int nil::objtype() const {
    return NIL_TYPE;
}

symbol::symbol(string sn) {
    symname = sn;
}

int symbol::objtype() const {
    return SYMBOL_TYPE;
}

string symbol::name() const {
    return symname;
}

cons::cons(shared_ptr<lispobj> a, shared_ptr<lispobj> d) {
    first = a;
    second = d;
}

int cons::objtype() const {
    return CONS_TYPE;
}

shared_ptr<lispobj> cons::car() const {
    return first;
}

shared_ptr<lispobj> cons::cdr() const {
    return second;
}

number::number(int n) {
    num = n;
}

int number::objtype() const {
    return NUMBER_TYPE;
}

int number::value() const {
    return num;
}

lispfunc::lispfunc(shared_ptr<lispobj> _args,
                   shared_ptr<environment> _closure,
                   shared_ptr<lispobj> _code) :
    args(_args),
    closure(_closure),
    code(_code)
{

}

int lispfunc::objtype() const {
    return FUNC_TYPE;
}

cfunc::cfunc(std::function<shared_ptr<lispobj>(vector<shared_ptr<lispobj> >)> f) :
    func(f)
{

}

int cfunc::objtype() const {
    return CFUNC_TYPE;
}

bool eq(shared_ptr<lispobj> left, shared_ptr<lispobj> right) {
    if(left == right) return true;
    if(left->objtype() != right->objtype()) return false;

    switch(left->objtype()) {
    case SYMBOL_TYPE:
    {
        shared_ptr<symbol> ls(std::dynamic_pointer_cast<symbol>(left));
        shared_ptr<symbol> rs(std::dynamic_pointer_cast<symbol>(right));
        return ls->name() == rs->name();
    }

    case NIL_TYPE:
        return true;

    default:
        return false;
    };
}

bool eqv(shared_ptr<lispobj> left, shared_ptr<lispobj> right) {
    if(eq(left, right)) return true;
    if(left->objtype() != right->objtype()) return false;

    switch(left->objtype()) {
    case NUMBER_TYPE:
    {
        shared_ptr<number> ln(std::dynamic_pointer_cast<number>(left));
        shared_ptr<number> rn(std::dynamic_pointer_cast<number>(right));
        return ln->value() && rn->value();
    }

    default:
        return false;
    };
}

bool equal(shared_ptr<lispobj> left, shared_ptr<lispobj> right) {
    if(eqv(left, right)) return true;
    if(left->objtype() != right->objtype()) return false;

    switch(left->objtype()) {
    case CONS_TYPE:
    {
        shared_ptr<cons> lc(std::dynamic_pointer_cast<cons>(left));
        shared_ptr<cons> rc(std::dynamic_pointer_cast<cons>(right));
        return equal(lc->car(), rc->car()) && equal(lc->cdr(), rc->cdr());
    }
    
    default:
        return false;
    };
}

void print(shared_ptr<lispobj> obj) {
    switch(obj->objtype()) {
    case NUMBER_TYPE:
    {
        shared_ptr<number> n(std::dynamic_pointer_cast<number>(obj));
        cout << n->value();
        break;
    }
    case SYMBOL_TYPE:
    {
        shared_ptr<symbol> s(std::dynamic_pointer_cast<symbol>(obj));
        cout << s->name();
        break;
    }
    case CONS_TYPE:
    {
        shared_ptr<cons> c(std::dynamic_pointer_cast<cons>(obj));
        cout << '(';
        print(c->car());
        obj = c->cdr();
        while(obj->objtype() == CONS_TYPE) {
            c = std::dynamic_pointer_cast<cons>(obj);
            cout << ' ';
            print(c->car());
            obj = c->cdr();
        }

        if(obj->objtype() == NIL_TYPE) {
            cout << ')';
        } else {
            cout << " . ";
            print(obj);
            cout << ')';
        }
        break;
    }
    case NIL_TYPE:
        cout << "'()";
        break;
    case CFUNC_TYPE:
        cout << "CFUNC";
        break;
    };
}

shared_ptr<lispobj> make_list(const vector<shared_ptr<lispobj> >& list_values) {
    shared_ptr<lispobj> ret(new nil());

    for(auto it = list_values.rbegin(); it != list_values.rend(); ++it) {
        ret.reset(new cons(*it, ret));
    }

    return ret;
}

shared_ptr<lispobj> _read(string& str) {
    // remove leading whitespace, could be faster, but wevs
    while(isspace(str[0])) {
        str.erase(0, 1);
    }

    // figure out type of object
    if(str[0] == '(') { //list
        str.erase(0, 1);
        vector<shared_ptr<lispobj> > list_values;

        while(str[0] != ')') {
            while(isspace(str[0])) {
                str.erase(0, 1);
            }

            list_values.push_back(_read(str));
        }

        str.erase(0, 1);
        return make_list(list_values);
    } else if (isdigit(str[0])) { // number ('.' too, once we have non-integers)
        size_t pos = 0;
        int n = std::stoi(str, &pos);
        str.erase(0, pos);
        return std::make_shared<number>(n);
    } else { //symbol
        int n = 1;
        while(!isspace(str[n]) && str[n] != '(' && str[n] != ')') {
            ++n;
        }

        string sym_name = str.substr(0, n);
        str.erase(0, n);
        return std::make_shared<symbol>(sym_name);
    }
}

shared_ptr<lispobj> read(string str) {
    return _read(str);
}

environment::environment() {

}

environment::environment(shared_ptr<environment> env) :
    parent(env)
{

}

void environment::set(string name, shared_ptr<lispobj> value) {
    // this is probably not the correct semantics when setting
    // a closed-upon value. ex:
    /*(define a 1)
      (define (incrementer)
        (set! a (+ a 1))) */
    // a would get set in function scope, not the parent scope.

    bindings[name] = value;
}

shared_ptr<lispobj> environment::get(string name) {
    auto it = bindings.find(name);
    if(it != bindings.end()) {
        return it->second;
    } else if(parent) {
        return parent->get(name);
    } else {
        //XXX: should probably be undefined or error
        return std::make_shared<nil>();
    }
}

class stackframe {
public:
    stackframe(shared_ptr<environment> e, int m, shared_ptr<lispobj> c) :
        env(e),
        mark(m),
        code(c)
    {}

    shared_ptr<environment> env;
    int mark;
    shared_ptr<lispobj> code;
    vector<shared_ptr<lispobj> > evaled_args;
};

const int evaluating = 0;
const int applying = 1;
const int evaled = 2;

void print_stack(const std::deque<stackframe> exec_stack) {
    cout << "Stack size: " << exec_stack.size() << endl;

    for(auto frame : exec_stack) {
        cout << "frame " << frame.mark << endl;
        cout << "  code: ";
        print(frame.code);
        cout << endl;
        cout << "  args:" << endl;
        for(auto arg: frame.evaled_args) {
            cout << "    ";
            print(arg);
            cout << endl;
        }
    }
}

shared_ptr<lispobj> eval(shared_ptr<lispobj> code, shared_ptr<environment> tle) {
    std::deque<stackframe> exec_stack;
    shared_ptr<lispobj> nil_obj(new nil());

    exec_stack.push_front(stackframe(tle, evaluating, code));

    while(exec_stack.size() > 1 || exec_stack.front().mark != evaled) {
        //print_stack(exec_stack);
        if(exec_stack.front().mark == evaled) {
            shared_ptr<lispobj> c = exec_stack.front().code;
            exec_stack.pop_front();
            if(exec_stack.front().mark == applying &&
               exec_stack.front().code->objtype() == NIL_TYPE) {
                exec_stack.front().mark = evaled;
                exec_stack.front().code = c;
            } else if(exec_stack.front().mark == evaluating) {
                exec_stack.front().evaled_args.push_back(c);
                //special forms go here?
                //if(exec_stack.front().evaled_args.size() == 1 &&
                //   it's a special form) {
                //   make special form happen }
            } else {
                cout << "ERROR: bad stack" << endl;
                return nullptr;
            }
        } else if(exec_stack.front().mark == applying) {
            if(code->objtype() == NIL_TYPE) {
                exec_stack.front().mark = evaled;
            } else if(code->objtype() == CONS_TYPE) {
                shared_ptr<cons> c = std::dynamic_pointer_cast<cons>(exec_stack.front().code);
                shared_ptr<lispobj> next_statement = c->car();
                exec_stack.front().code = c->cdr();
                exec_stack.push_front(stackframe(exec_stack.front().env,
                                                 evaluating,
                                                 next_statement));
            } else {
                cout << "ERROR: bad stack" << endl;
                return nullptr;
            }
        } else if(exec_stack.front().mark == evaluating) {
            if(exec_stack.front().code->objtype() == CONS_TYPE) {
                //evaluating arguments
                shared_ptr<cons> c = std::dynamic_pointer_cast<cons>(exec_stack.front().code);
                exec_stack.front().code = c->cdr();
                exec_stack.push_front(stackframe(exec_stack.front().env,
                                                 evaluating,
                                                 c->car()));
            } else if(exec_stack.front().code->objtype() == NIL_TYPE) {
                auto evaled_args = exec_stack.front().evaled_args;
                if(evaled_args.empty()) {
                    cout << "ERROR: empty function application" << endl;
                    return nullptr;
                } else if(evaled_args.front()->objtype() == FUNC_TYPE) {
                    shared_ptr<lispfunc> func = std::dynamic_pointer_cast<lispfunc>(evaled_args.front());
                    evaled_args.erase(evaled_args.begin());
                    shared_ptr<environment> env(new environment(func->closure));

                    auto arg_value_iter = evaled_args.begin();
                    shared_ptr<lispobj> arg_names = func->args;
                    while(arg_value_iter != evaled_args.end() &&
                          arg_names->objtype() == CONS_TYPE) {
                        shared_ptr<cons> c = std::dynamic_pointer_cast<cons>(arg_names);
                        shared_ptr<lispobj> name = c->car();
                        if(name->objtype() == SYMBOL_TYPE) {
                            shared_ptr<symbol> s = std::dynamic_pointer_cast<symbol>(name);
                            env->set(s->name(), *arg_value_iter);
                        } else {
                            cout << "ERROR: arguments must be symbols" << endl;
                            return nullptr;
                        }
                        arg_names = c->cdr();
                    }

                    if(arg_names->objtype() != NIL_TYPE ||
                       arg_value_iter != evaled_args.end()) {
                        cout << "ERROR: function arity does not match call." << endl;
                        return nullptr;
                    }

                    exec_stack.front().mark = applying;
                    exec_stack.front().code = func->code;
                } else if(evaled_args.front()->objtype() == CFUNC_TYPE) {
                    shared_ptr<cfunc> func = std::dynamic_pointer_cast<cfunc>(evaled_args.front());
                    evaled_args.erase(evaled_args.begin());
                    shared_ptr<lispobj> ret = func->func(evaled_args);
                    return ret;
                } else {
                    cout << "ERROR: trying to apply a non-function" << endl;
                    return nullptr;
                }
            } else if(exec_stack.front().code->objtype() == SYMBOL_TYPE) {
                //variable lookup
                exec_stack.front().mark = evaled;
                shared_ptr<symbol> s = std::dynamic_pointer_cast<symbol>(exec_stack.front().code);
                exec_stack.front().code = exec_stack.front().env->get(s->name());
            } else {
                //constant value
                exec_stack.front().mark = evaled;
            }
        } else {
            cout << "ERROR: bad stack" << endl;
            return nullptr;
        }
    }

    return exec_stack.front().code;
}

shared_ptr<lispobj> plus(vector<shared_ptr<lispobj> > args) {
    int sum = 0;
    for(auto obj : args) {
        if(obj->objtype() != NUMBER_TYPE) {
            cout << "plus requires numbers" << endl;
            return nullptr;
        }
        shared_ptr<number> n = std::dynamic_pointer_cast<number>(obj);
        sum += n->value();
    }
    return std::make_shared<number>(sum);
}

shared_ptr<lispobj> minus(vector<shared_ptr<lispobj> > args) {
    if(args.size() > 1) {
        if(args[0]->objtype() != NUMBER_TYPE) {
            cout << "minus requires numbers" << endl;
            return nullptr;
        }
        shared_ptr<number> n = std::dynamic_pointer_cast<number>(args[0]);
        int diff = n->value();
        args.erase(args.begin());

        for(auto obj : args) {
            if(obj->objtype() != NUMBER_TYPE) {
                cout << "plus requires numbers" << endl;
                return nullptr;
            }
            shared_ptr<number> n = std::dynamic_pointer_cast<number>(obj);
            diff -= n->value();
        }
        return std::make_shared<number>(diff);
    } else {
        shared_ptr<lispobj> obj(args[0]);
        if(obj->objtype() != NUMBER_TYPE) {
            cout << "minus requires numbers" << endl;
        }
        shared_ptr<number> n = std::dynamic_pointer_cast<number>(obj);
        return std::make_shared<number>(-(n->value()));
    }
}

shared_ptr<lispobj> multiply(vector<shared_ptr<lispobj> > args) {
    int product = 1;
    for(auto obj : args) {
        if(obj->objtype() != NUMBER_TYPE) {
            cout << "plus requires numbers" << endl;
            return nullptr;
        }
        shared_ptr<number> n = std::dynamic_pointer_cast<number>(obj);
        product *= n->value();
    }
    return std::make_shared<number>(product);
}

shared_ptr<environment> make_standard_env() {
    shared_ptr<environment> env(new environment());
    env->set("+", std::make_shared<cfunc>(cfunc(plus)));
    env->set("-", std::make_shared<cfunc>(cfunc(minus)));
    env->set("*", std::make_shared<cfunc>(cfunc(multiply)));
    return env;
}
