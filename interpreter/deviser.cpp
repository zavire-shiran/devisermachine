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
                   shared_ptr<lexicalscope> _closure,
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

module::module(shared_ptr<lispobj> _name, shared_ptr<environment> env) :
    name(_name),
    scope(new lexicalscope(env)),
    inited(false)
{

}

void module::add_import(shared_ptr<lispobj> modname) {
    imports.push_back(modname);
}

void module::add_export(shared_ptr<symbol> sym) {
    exports.push_back(sym);
}

void module::define(string name, shared_ptr<lispobj> value) {
    scope->define(name, value);
}

void module::add_init(shared_ptr<lispobj> initblock) {
    initblocks.push_back(initblock);
}

shared_ptr<lispobj> module::get_name() const {
    return name;
}

shared_ptr<lexicalscope> module::get_bindings() const {
    return scope;
}

int module::objtype() const {
    return MODULE_TYPE;
}

bool module::init(shared_ptr<environment> env) {
    if(inited) {
        return true;;
    }

    for(auto imp : imports) {
        shared_ptr<module> mod = env->get_module_by_prefix(imp);
        mod->init(env);
        scope->add_import(mod);
    }

    // it would be good if this could go on the stack of the currently running eval
    // but that's ~*~hard~*~, and this is easy
    for(auto initblock : initblocks) {
        shared_ptr<lexicalscope> initscope(new lexicalscope(env, scope));
        if(eval(std::make_shared<cons>(std::make_shared<symbol>("begin"), initblock),
                initscope, env) == nullptr) {
            return false;
        }
    }

    inited = true;
    return true;
}

const vector< shared_ptr<lispobj> >& module::get_imports() const {
    return imports;
}

const vector< shared_ptr<symbol> >& module::get_exports() const {
    return exports;
}

const vector< shared_ptr<lispobj> >& module::get_initblocks() const {
    return initblocks;
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
    case MODULE_TYPE:
    {
        shared_ptr<module> mod = std::dynamic_pointer_cast<module>(obj);
        cout << "(module ";
        print(mod->get_name());
        cout << " (imports ";
        auto imports = mod->get_imports();
        for(auto import : imports) {
            print(import) ;
            cout << " ";
        }
        cout << ") (exports ";
        auto exports = mod->get_exports();
        for(auto exprt : exports) {
            print(exprt);
            cout << " ";
        }
        cout << ") (init ";
        auto inits = mod->get_initblocks();
        for(auto init : inits) {
            print(init);
            cout  << " ";
        }
        cout << ") ";
        mod->get_bindings()->dump();
        cout << ")";
        break;
    }
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

    if(str[0] == ')') {
        return nullptr;
    }

    // figure out type of object
    if(str[0] == '(') { //list
        str.erase(0, 1);
        vector<shared_ptr<lispobj> > list_values;

        while(str[0] != ')') {
            while(isspace(str[0])) {
                str.erase(0, 1);
            }

            auto obj = _read(str);
            if(obj) {
               list_values.push_back(obj);
            }
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

vector< shared_ptr<lispobj> > readall(string str) {
    vector< shared_ptr<lispobj> > ret;

    while(!str.empty()) {
        shared_ptr<lispobj> lobj = _read(str);
        if(lobj) {
            ret.push_back(lobj);
        }
    }

    return ret;
}

environment::environment() :
    scope(nullptr)
{

}

shared_ptr<lexicalscope> environment::get_scope() {
    return scope;
}

void environment::set_scope(shared_ptr<lexicalscope> s) {
    scope = s;
}

void environment::add_module_def(shared_ptr<module> m) {
    modules.push_back(m);
}

bool prefix_match(shared_ptr<lispobj> name, shared_ptr<lispobj> prefix) {
    if(prefix->objtype() == NIL_TYPE || eqv(name, prefix)) {
        return true;
    }

    if(name->objtype() != prefix->objtype()) {
        return false;
    }

    if(name->objtype() == CONS_TYPE) {
        shared_ptr<cons> name_cons = std::dynamic_pointer_cast<cons>(name);
        shared_ptr<cons> prefix_cons = std::dynamic_pointer_cast<cons>(prefix);
        return prefix_match(name_cons->car(), prefix_cons->car()) &&
            prefix_match(name_cons->cdr(), prefix_cons->cdr());
    }

    cout << "Invalid type in module name: ";
    print(name);
    cout << endl;
    return false;
}

shared_ptr<module> environment::get_module_by_prefix(shared_ptr<lispobj> prefix) {
    for(auto mod : modules) {
        if(prefix_match(mod->get_name(), prefix)) {
            return mod;
        }
    }

    return nullptr;
}

lexicalscope::lexicalscope(shared_ptr<environment> e) :
    parent(nullptr),
    env(e)
{

}

lexicalscope::lexicalscope(shared_ptr<environment> e, shared_ptr<lexicalscope> p) :
    parent(p),
    env(e)
{

}

void lexicalscope::define(string name, shared_ptr<lispobj> value) {
    // shadow any bindings in parent scopes, but don't modify them.
    // maybe should error when name already exists
    bindings[name] = value;
}

void lexicalscope::set(string name, shared_ptr<lispobj> value) {
    // try to find a variable binding. if it already exists,
    // set it in the same lexicalscope that we found it
    shared_ptr<lexicalscope> scope = shared_ptr<lexicalscope>(this);
    while(scope != nullptr) {
        auto iter = scope->bindings.find(name);
        if(iter != scope->bindings.end()) {
            iter->second = value;
            return;
        }
        scope = scope->parent;
    }

    // it has not been set yet, so set it in the current frame
    bindings[name] = value;
}

shared_ptr<lispobj> lexicalscope::get(string name) {
    auto it = bindings.find(name);
    if(it != bindings.end()) {
        return it->second;
    } else {
        for(shared_ptr<module> mod : imports) {
            for(shared_ptr<symbol> sym : mod->get_exports()) {
                if(sym->name() == name) {
                    return mod->get_bindings()->get(name);
                }
            }
        }

        if(parent) {
            return parent->get(name);
        } else {
            //XXX: should probably be undefined or error
            return std::make_shared<nil>();
        }
    }
}

void lexicalscope::add_import(shared_ptr<module> mod) {
    imports.push_back(mod);
}

const vector< shared_ptr<module> >& lexicalscope::get_imports() const {
    return imports;
}

void lexicalscope::dump() {
    for(auto it = bindings.begin(); it != bindings.end(); ++it) {
        cout << it->first << ": ";
        print(it->second);
        cout<< endl;
    }

    if(parent) {
        cout << "parent lexicalscope:" << endl;
        parent->dump();
    }
}

class stackframe {
public:
    stackframe(shared_ptr<lexicalscope> s, int m, shared_ptr<lispobj> c) :
        scope(s),
        mark(m),
        code(c)
    {}

    shared_ptr<lexicalscope> scope;
    int mark;
    shared_ptr<lispobj> code;
    vector<shared_ptr<lispobj> > evaled_args;
};

const int evaluating = 0;
const int applying = 1;
const int evaled = 2;
const int evalspecial = 3;

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

int apply_lispfunc(std::deque<stackframe>& exec_stack, shared_ptr<environment> env) {
    auto evaled_args = exec_stack.front().evaled_args;
    shared_ptr<lispfunc> func = std::dynamic_pointer_cast<lispfunc>(evaled_args.front());
    evaled_args.erase(evaled_args.begin());
    shared_ptr<lexicalscope> scope(new lexicalscope(env, func->closure));

    auto arg_value_iter = evaled_args.begin();
    shared_ptr<lispobj> arg_names = func->args;
    while(arg_value_iter != evaled_args.end() &&
          arg_names->objtype() == CONS_TYPE) {
        shared_ptr<cons> c = std::dynamic_pointer_cast<cons>(arg_names);
        shared_ptr<lispobj> name = c->car();
        if(name->objtype() == SYMBOL_TYPE) {
            shared_ptr<symbol> s = std::dynamic_pointer_cast<symbol>(name);
            scope->define(s->name(), *arg_value_iter);
        } else {
            cout << "ERROR: arguments must be symbols" << endl;
            return 1;
        }
        arg_names = c->cdr();
        ++arg_value_iter;
    }

    if(arg_names->objtype() != NIL_TYPE ||
       arg_value_iter != evaled_args.end()) {
        cout << "ERROR: function arity does not match call." << endl;
        print(func->args); cout << endl;
        for(auto arg_value_iter = evaled_args.begin();
            arg_value_iter != evaled_args.end();
            ++arg_value_iter) {
            print(*arg_value_iter); cout << " ";
        }
        cout << endl;
        print(arg_names); cout << endl;
        return 1;
    }

    exec_stack.front().mark = applying;
    exec_stack.front().scope = scope;
    exec_stack.front().code = func->code;
    return 0;
}

bool is_special_form(shared_ptr<lispobj> form) {
    if(form->objtype() == SYMBOL_TYPE) {
        shared_ptr<symbol> sym = std::dynamic_pointer_cast<symbol>(form);
        if(sym->name() == "if") {
            return true;
        } else if(sym->name() == "lambda") {
            return true;
        } else if(sym->name() == "module") {
            return true;
        } else if(sym->name() == "import") {
            return true;
        } else if(sym->name() == "begin") {
            return true;
        }
    }
    return false;
}

int add_import_to_module(shared_ptr<module> mod, shared_ptr<lispobj> importdecl) {
    if(importdecl->objtype() != CONS_TYPE) {
        return 1;
    }

    shared_ptr<cons> c = std::dynamic_pointer_cast<cons>(importdecl);
    if(c->cdr()->objtype() != NIL_TYPE) {
        cout << "too many arguments to import" << endl;
        return 1;
    }

    mod->add_import(c->car());
    return 0;
}

int add_export_to_module(shared_ptr<module> mod, shared_ptr<lispobj> exportdecl) {
    if(exportdecl->objtype() != CONS_TYPE) {
        cout << "nothing to export" << endl;
        return 1;
    }

    while(exportdecl->objtype() == CONS_TYPE) {
        shared_ptr<cons> c = std::dynamic_pointer_cast<cons>(exportdecl);
        if(c->car()->objtype() != SYMBOL_TYPE) {
            cout << "non-symbol in export list" << endl;
            return 1;
        }

        mod->add_export(std::dynamic_pointer_cast<symbol>(c->car()));

        exportdecl = c->cdr();
    }

    return 0;
}

int add_define_to_module(shared_ptr<module> mod,
                         shared_ptr<lispobj> definedecl,
                         shared_ptr<lexicalscope> scope,
                         shared_ptr<environment> env) {
    if(definedecl->objtype() != CONS_TYPE) {
        return 1;
    }

    shared_ptr<cons> c = std::dynamic_pointer_cast<cons>(definedecl);
    if(c->car()->objtype() != SYMBOL_TYPE){
        return 1;
    }

    shared_ptr<symbol> defname = std::dynamic_pointer_cast<symbol>(c->car());

    if(c->cdr()->objtype() != CONS_TYPE) {
        return 1;
    }

    c = std::dynamic_pointer_cast<cons>(c->cdr());
    shared_ptr<lispobj> val = eval(c->car(), scope, env);
    if(val) {
        mod->define(defname->name(), val);
    } else {
        cout << "error evaluating define for module" << endl;
        return 1;
    }

    return 0;
}

int eval_module_special_form(std::deque<stackframe>& exec_stack,
                             shared_ptr<environment> env) {
    if(exec_stack.front().code->objtype() != CONS_TYPE) {
        cout << "module does not have name" << endl;
        return 1;
    }
    shared_ptr<lispobj> lobj;
    shared_ptr<cons> c = std::dynamic_pointer_cast<cons>(exec_stack.front().code);
    shared_ptr<module> m(new module(c->car(), env));

    lobj = c->cdr();
    while(lobj->objtype() == CONS_TYPE) {
        c = std::dynamic_pointer_cast<cons>(lobj);

        if(c->car()->objtype() != CONS_TYPE) {
            cout << "invalid module declaration:";
            print(c);
            cout << endl;
            return 1;
        }

        shared_ptr<cons> decl = std::dynamic_pointer_cast<cons>(c->car());

        if(decl->car()->objtype() != SYMBOL_TYPE) {
            cout << "invalid module declaration:";
            print(c);
            cout << endl;
            return 1;
        }

        shared_ptr<symbol> s = std::dynamic_pointer_cast<symbol>(decl->car());
        if(s->name() == "import") {
            if(add_import_to_module(m, decl->cdr())) {
                cout << "invalid import declaration: ";
                print(decl);
                cout << endl;
                return 1;
            }
        } else if(s->name() == "export") {
            if(add_export_to_module(m, decl->cdr())) {
                cout << "invalid export declaration: ";
                print(decl);
                cout << endl;
                return 1;
            }
        } else if(s->name() == "define") {
            if(add_define_to_module(m, decl->cdr(), exec_stack.front().scope, env)) {
                cout << "invalid define declaration: ";
                print (decl);
                cout << endl;
                return 1;
            }
        } else if(s->name() == "init") {
            m->add_init(decl->cdr());
        }

        lobj = c->cdr();
    }

    exec_stack.front().mark = evaled;
    exec_stack.front().code = m;

    return 0;
}

int eval_if_special_form(std::deque<stackframe>& exec_stack) {
    if(exec_stack.front().evaled_args.size() == 1) {
        shared_ptr<lispobj> lobj = exec_stack.front().code;
        if(lobj->objtype() != CONS_TYPE) {
            print(lobj);
            cout << " if has no condition" << endl;
            return 1;
        }
        shared_ptr<cons> c = std::dynamic_pointer_cast<cons>(lobj);
        exec_stack.front().code = c->cdr();
        exec_stack.push_front(stackframe(exec_stack.front().scope,
                                         evaluating,
                                         c->car()));
    } else if(exec_stack.front().evaled_args.size() == 2) {
        if(exec_stack.front().evaled_args[1]->objtype() == NIL_TYPE) {
            if(exec_stack.front().code->objtype() != CONS_TYPE) {
                cout << "if has no true branch" << endl;
                return 1;
            }
            shared_ptr<cons> c = std::dynamic_pointer_cast<cons>(exec_stack.front().code);
            if(c->cdr()->objtype() != CONS_TYPE) {
                exec_stack.front().mark = evaled;
                exec_stack.front().code = std::make_shared<nil>();
                exec_stack.front().evaled_args.clear();
            } else {
                shared_ptr<cons> c2 = std::dynamic_pointer_cast<cons>(c->cdr());
                exec_stack.front().mark = evaluating;
                exec_stack.front().code = c2->car();
                exec_stack.front().evaled_args.clear();
            }
        } else {
            if(exec_stack.front().code->objtype() != CONS_TYPE) {
                cout << "if has no true branch" << endl;
                return 1;
            }
            shared_ptr<cons> c = std::dynamic_pointer_cast<cons>(exec_stack.front().code);
            exec_stack.front().mark = evaluating;
            exec_stack.front().code = c->car();
            exec_stack.front().evaled_args.clear();
        }
    }

    return 0;
}

int eval_special_form(string name,
                      std::deque<stackframe>& exec_stack,
                      shared_ptr<environment> env) {
    if(name == "if") {
        return eval_if_special_form(exec_stack);
    } else if(name == "lambda") {
        shared_ptr<lispobj> lobj = exec_stack.front().code;
        if(lobj->objtype() != CONS_TYPE) {
            cout << "lambda needs more arguments" << endl;
            return 1;
        }

        shared_ptr<cons> c = std::dynamic_pointer_cast<cons>(lobj);
        if(c->cdr()->objtype() != CONS_TYPE) {
            cout << "lambda needs more arguments" << endl;
            return 1;
        }
        shared_ptr<cons> c2 = std::dynamic_pointer_cast<cons>(c->cdr());
        shared_ptr<lispfunc> lfunc(new lispfunc(c->car(), exec_stack.front().scope, c2));
        exec_stack.front().mark = evaled;
        exec_stack.front().code = lfunc;
    } else if(name == "module") {
        return eval_module_special_form(exec_stack, env);
    } else if(name == "import") {
        shared_ptr<lispobj> lobj = exec_stack.front().code;
        if(lobj->objtype() != CONS_TYPE) {
            cout << "import needs more arguments" << endl;
            return 1;
        }

        shared_ptr<cons> c = std::dynamic_pointer_cast<cons>(lobj);
        shared_ptr<module> m = env->get_module_by_prefix(c->car());
        if(!(m->init(env))) {
            return 1;
        }
        exec_stack.front().scope->add_import(m);
    } else if(name == "begin") {
        exec_stack.front().mark = applying;
    }
    return 0;
}

shared_ptr<lispobj> eval(shared_ptr<lispobj> code,
                         shared_ptr<lexicalscope> tls,
                         shared_ptr<environment> env) {
    std::deque<stackframe> exec_stack;
    shared_ptr<lispobj> nil_obj(new nil());

    exec_stack.push_front(stackframe(tls, evaluating, code));

    while(exec_stack.size() > 1 || exec_stack.front().mark != evaled) {
        //print_stack(exec_stack);
        if(exec_stack.front().mark == evaled) {
            shared_ptr<lispobj> c = exec_stack.front().code;
            exec_stack.pop_front();
            if(exec_stack.front().mark == applying) {
                if(exec_stack.front().code->objtype() == NIL_TYPE) {
                    exec_stack.front().mark = evaled;
                    exec_stack.front().code = c;
                } else {
                    //the frame was popped, and nothing else needs doing
                }
            } else if(exec_stack.front().mark == evaluating ||
                      exec_stack.front().mark == evalspecial) {
                exec_stack.front().evaled_args.push_back(c);
            } else {
                cout << "ERROR: bad stack" << endl;
                return nullptr;
            }
        } else if(exec_stack.front().mark == applying) {
            if(exec_stack.front().code->objtype() == NIL_TYPE) {
                exec_stack.front().mark = evaled;
            } else if(exec_stack.front().code->objtype() == CONS_TYPE) {
                shared_ptr<cons> c = std::dynamic_pointer_cast<cons>(exec_stack.front().code);
                shared_ptr<lispobj> next_statement = c->car();
                exec_stack.front().code = c->cdr();
                exec_stack.push_front(stackframe(exec_stack.front().scope,
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
                //special forms go here, rather than normal argument evaluation
                if(exec_stack.front().evaled_args.size() == 0 &&
                   is_special_form(c->car())) {
                    exec_stack.front().mark = evalspecial;
                    exec_stack.front().evaled_args.push_back(c->car());
                    exec_stack.front().code = c->cdr();
                } else {
                    exec_stack.front().code = c->cdr();
                    exec_stack.push_front(stackframe(exec_stack.front().scope,
                                                     evaluating,
                                                     c->car()));
                }
            } else if(exec_stack.front().code->objtype() == NIL_TYPE) {
                auto evaled_args = exec_stack.front().evaled_args;
                if(evaled_args.empty()) {
                    cout << "ERROR: empty function application" << endl;
                    return nullptr;
                } else if(evaled_args.front()->objtype() == FUNC_TYPE) {
                    if(apply_lispfunc(exec_stack, env) != 0) {
                        return nullptr;
                    }
                } else if(evaled_args.front()->objtype() == CFUNC_TYPE) {
                    shared_ptr<cfunc> func = std::dynamic_pointer_cast<cfunc>(evaled_args.front());
                    evaled_args.erase(evaled_args.begin());
                    shared_ptr<lispobj> ret = func->func(evaled_args);
                    exec_stack.front().mark = evaled;
                    exec_stack.front().code = ret;
                    exec_stack.front().evaled_args.clear();
                } else {
                    cout << "ERROR: trying to apply a non-function" << endl;
                    return nullptr;
                }
            } else if(exec_stack.front().code->objtype() == SYMBOL_TYPE) {
                //variable lookup
                exec_stack.front().mark = evaled;
                shared_ptr<symbol> s = std::dynamic_pointer_cast<symbol>(exec_stack.front().code);
                if(s->name() == "nil") {
                    exec_stack.front().code = nil_obj;
                } else {
                    exec_stack.front().code = exec_stack.front().scope->get(s->name());
                }
                //cout << "getting var " << s->name() << ": ";
                //print(exec_stack.front().code); cout << endl;
            } else {
                //constant value
                exec_stack.front().mark = evaled;
            }
        } else if(exec_stack.front().mark == evalspecial) {
            shared_ptr<lispobj> list_head = exec_stack.front().evaled_args[0];
            if(list_head->objtype() != SYMBOL_TYPE) {
                cout << "non-special form given evalspecial mark." << endl;
                return nullptr;
            }
            shared_ptr<symbol> sym = std::dynamic_pointer_cast<symbol>(list_head);
            if(eval_special_form(sym->name(), exec_stack, env) != 0) {
                return nullptr;
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
                cout << "minus requires numbers" << endl;
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

shared_ptr<lispobj> divide(vector<shared_ptr<lispobj> > args) {
    if(args.size() > 1) {
        if(args[0]->objtype() != NUMBER_TYPE) {
            cout << "divide requires numbers" << endl;
            return nullptr;
        }
        shared_ptr<number> n = std::dynamic_pointer_cast<number>(args[0]);
        int quotient = n->value();
        args.erase(args.begin());

        for(auto obj : args) {
            if(obj->objtype() != NUMBER_TYPE) {
                cout << "divide requires numbers" << endl;
                return nullptr;
            }
            shared_ptr<number> n = std::dynamic_pointer_cast<number>(obj);
            quotient /= n->value();
        }
        return std::make_shared<number>(quotient);
    } else {
        shared_ptr<lispobj> obj(args[0]);
        if(obj->objtype() != NUMBER_TYPE) {
            cout << "divide requires numbers" << endl;
        }
        shared_ptr<number> n = std::dynamic_pointer_cast<number>(obj);
        return std::make_shared<number>(n->value());
    }
}

shared_ptr<environment> make_standard_env() {
    shared_ptr<environment> env(new environment());
    env->set_scope(std::make_shared<lexicalscope>(env));
    env->get_scope()->define("+", std::make_shared<cfunc>(cfunc(plus)));
    env->get_scope()->define("-", std::make_shared<cfunc>(cfunc(minus)));
    env->get_scope()->define("*", std::make_shared<cfunc>(cfunc(multiply)));
    env->get_scope()->define("/", std::make_shared<cfunc>(cfunc(divide)));
    return env;
}
