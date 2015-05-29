#include <istream>
#include <iostream>
#include <stack>
#include "deviser.hpp"

using std::cout;
using std::endl;
using std::dynamic_pointer_cast;
using std::make_shared;

lispobj::lispobj() {}

nil::nil() {}

int nil::objtype() const {
    return NIL_TYPE;
}

void nil::print() {
    cout << "'()";
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

void symbol::print() {
    cout << symname;
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

void cons::print() {
    cout << '(';
    car()->print();
    shared_ptr<cons> c;
    shared_ptr<lispobj> obj = cdr();
    while(obj->objtype() == CONS_TYPE) {
        c = dynamic_pointer_cast<cons>(obj);
        cout << ' ';
        c->car()->print();
        obj = c->cdr();
    }

    if(obj->objtype() == NIL_TYPE) {
        cout << ')';
    } else {
        cout << " . ";
        obj->print();
        cout << ')';
    }
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

void number::print() {
    cout << value();
}

lispstring::lispstring() {

}

lispstring::lispstring(const string& str) :
    contents(str)
{

}

void lispstring::append(shared_ptr<lispstring> lstr) {
    contents.append(lstr->contents);
}

const string& lispstring::get_contents() const {
    return contents;
}

int lispstring::objtype() const {
    return STRING_TYPE;
}

void lispstring::print() {
    cout << "\"";
    for(auto it = contents.begin(); it != contents.end(); ++it) {
        switch(*it) {
        case '"':
            cout << "\\\"";
            break;
        case '\\':
            cout << "\\\\";
            break;
        case '\n':
            cout << "\\n";
            break;
        case '\t':
            cout << "\\t";
            break;
        default:
            cout << *it;
            break;
        }
    }
    cout << "\"";
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

void lispfunc::print() {
    cout << "(lambda ";
    args->print();
    cout << " ";
    args->print();
    cout << ")";
}

cfunc::cfunc(std::function<shared_ptr<lispobj>(vector<shared_ptr<lispobj> >)> f) :
    func(f)
{

}

int cfunc::objtype() const {
    return CFUNC_TYPE;
}

void cfunc::print() {
    cout << "CFUNC";
}

fileinputport::fileinputport(string fname) :
    filename(fname),
    instream(filename)
{

}

shared_ptr<lispobj> fileinputport::read() {
    char buf[256];
    instream.read(buf, 256);
    if(instream) {
        return make_shared<lispstring>(buf);
    } else {
        return make_shared<lispstring>();
    }
}

shared_ptr<lispobj> fileinputport::readchar() {
    char buf;
    instream.get(buf);
    if(instream) {
        return make_shared<lispstring>(string(&buf, 1));
    } else {
        return make_shared<lispstring>();
    }
}

int fileinputport::objtype() const {
    return FILEINPUTPORT_TYPE;
}

void fileinputport::print() {
    cout << "<fileinputport: " << filename << ">";
}


module::module(shared_ptr<lispobj> _name) :
    name(_name),
    scope(new lexicalscope()),
    inited(false)
{

}

string get_command_name(shared_ptr<lispobj> command) {
    if(command->objtype() != CONS_TYPE) {
        return "";
    }

    shared_ptr<cons> c = dynamic_pointer_cast<cons>(command);
    if(c->car()->objtype() != SYMBOL_TYPE) {
        return "";
    }

    shared_ptr<symbol> sym = dynamic_pointer_cast<symbol>(c->car());
    return sym->name();
}

shared_ptr<lispobj> module::eval(shared_ptr<lispobj> command) {
    string command_name = get_command_name(command);

    if(command_name == "define") {
        defines.push_back(command);
        return ::eval(command, scope);
    } else if(command_name == "undefine") {

    } else if(command_name == "export") {
        shared_ptr<cons> command_cons = dynamic_pointer_cast<cons>(command);
        shared_ptr<lispobj> exportdecl = command_cons->cdr();

        if(exportdecl->objtype() != CONS_TYPE) {
            cout << "nothing to export" << endl;
            return make_shared<nil>();
        }

        while(exportdecl->objtype() == CONS_TYPE) {
            shared_ptr<cons> c = dynamic_pointer_cast<cons>(exportdecl);
            if(c->car()->objtype() != SYMBOL_TYPE) {
                cout << "non-symbol in export list" << endl;
                return make_shared<nil>();
            }

            add_export(dynamic_pointer_cast<symbol>(c->car()));

            exportdecl = c->cdr();
        }
        return command_cons->cdr();
    } else if(command_name == "unexport") {

    } else if(command_name == "import") {
        shared_ptr<cons> command_cons = dynamic_pointer_cast<cons>(command);
        shared_ptr<lispobj> importdecl = command_cons->cdr();

        if(importdecl->objtype() != CONS_TYPE) {
            return make_shared<nil>();
        }

        shared_ptr<cons> c = dynamic_pointer_cast<cons>(importdecl);
        if(c->cdr()->objtype() != NIL_TYPE) {
            cout << "too many arguments to import" << endl;
            return make_shared<nil>();
        }

//        add_import(c->car());
        return c->car();
    } else if(command_name == "unimport") {

    } else if(command_name == "init") {
        shared_ptr<cons> command_cons = dynamic_pointer_cast<cons>(command);
        initblocks.push_back(command_cons->cdr());
    } else if(command_name == "dump") {
        cout << "(module ";
        name->print();

        if(imports.size() > 0) {
            cout << endl;
            cout << "  ";
            make_shared<cons>(make_shared<symbol>("import"),
                              make_reverse_list(imports.rbegin(),
                                                imports.rend()))->print();
        }

        if(exports.size() > 0) {
            cout << endl;
            cout << "  ";
            make_shared<cons>(make_shared<symbol>("export"),
                              make_reverse_list(exports.rbegin(),
                                                exports.rend()))->print();
        }

        for(auto define : defines) {
            cout << endl;
            cout << "  ";
            define->print();
        }

        for(auto initblock : initblocks) {
            cout << endl;
            cout << "  ";
            make_shared<cons>(make_shared<symbol>("init"), initblock)->print();
        }

        cout << ")" << endl;
    } else {
        return ::eval(command, scope);
    }

    return make_shared<nil>();
}

void module::add_import(shared_ptr<module> mod) {
    imports.push_back(mod->get_name());
    scope->add_import(mod);
}

void module::add_export(shared_ptr<symbol> sym) {
    exports.push_back(sym);
}

void module::defval(string name, shared_ptr<lispobj> value) {
    scope->defval(name, value);
}

void module::defun(string name, shared_ptr<lispobj> value) {
    scope->defun(name, value);
}

void module::defval_and_export(string symname, shared_ptr<lispobj> value) {
    shared_ptr<symbol> name(new symbol(symname));
    defval(symname, value);
    add_export(name);
}

void module::defun_and_export(string symname, shared_ptr<lispobj> value) {
    shared_ptr<symbol> name(new symbol(symname));
    defun(symname, value);
    add_export(name);
}

void module::add_init(shared_ptr<lispobj> initblock) {
    initblocks.push_back(initblock);
}

shared_ptr<lispobj> module::get_name() const {
    return name;
}

void module::set_name(shared_ptr<lispobj> newname) {
    name = newname;
}

shared_ptr<lexicalscope> module::get_bindings() const {
    return scope;
}

int module::objtype() const {
    return MODULE_TYPE;
}

bool module::init() {
    if(inited) {
        return true;;
    }

    for(auto imp : imports) {
        shared_ptr<module> mod = nullptr; //env->get_module_by_prefix(imp);
        mod->init();
        scope->add_import(mod);
    }

    // it would be good if this could go on the stack of the currently running eval
    // but that's ~*~hard~*~, and this is easy
    for(auto initblock : initblocks) {
        shared_ptr<lexicalscope> initscope(new lexicalscope(scope));
        if(::eval(make_shared<cons>(make_shared<symbol>("begin"), initblock),
                  initscope) == nullptr) {
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

void module::print() {
    cout << "(module ";
    get_name()->print();
    cout << " (imports ";
    auto imports = get_imports();
    for(auto import : imports) {
        import->print();
        cout << " ";
    }
    cout << ") (exports ";
    auto exports = get_exports();
    for(auto exprt : exports) {
        exprt->print();
        cout << " ";
    }
    cout << ") (init ";
    auto inits = get_initblocks();
    for(auto init : inits) {
        init->print();
        cout  << " ";
    }
    cout << ") ";
    get_bindings()->dump();
    cout << ")";
}

bool eq(shared_ptr<lispobj> left, shared_ptr<lispobj> right) {
    if(left == right) return true;
    if(left->objtype() != right->objtype()) return false;

    switch(left->objtype()) {
    case SYMBOL_TYPE:
    {
        shared_ptr<symbol> ls(dynamic_pointer_cast<symbol>(left));
        shared_ptr<symbol> rs(dynamic_pointer_cast<symbol>(right));
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
        shared_ptr<number> ln(dynamic_pointer_cast<number>(left));
        shared_ptr<number> rn(dynamic_pointer_cast<number>(right));
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
        shared_ptr<cons> lc(dynamic_pointer_cast<cons>(left));
        shared_ptr<cons> rc(dynamic_pointer_cast<cons>(right));
        return equal(lc->car(), rc->car()) && equal(lc->cdr(), rc->cdr());
    }
    
    default:
        return false;
    };
}

void printall(vector< shared_ptr<lispobj> > objs) {
    for(auto obj : objs) {
        obj->print();
        cout << endl;
    }
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
        return make_reverse_list(list_values.rbegin(), list_values.rend());
    } else if(isdigit(str[0])) { // number ('.' too, once we have non-integers)
        size_t pos = 0;
        int n = std::stoi(str, &pos);
        str.erase(0, pos);
        return make_shared<number>(n);
    } else if(str[0] == '"') {
        string contents;
        int n = 1;
        while(str[n] != '"') {
            if(str[n] == '\\') {
                contents.push_back(str[n+1]);
                ++n;
            } else {
                contents.push_back(str[n]);
            }
            ++n;
        }

        str.erase(0,n+1);
        return make_shared<lispstring>(contents);
    } else { //symbol
        int n = 1;
        while(!isspace(str[n]) && str[n] != '(' && str[n] != ')') {
            ++n;
        }

        string sym_name = str.substr(0, n);
        str.erase(0, n);
        return make_shared<symbol>(sym_name);
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

bool prefix_match(shared_ptr<lispobj> name, shared_ptr<lispobj> prefix) {
    if(prefix->objtype() == NIL_TYPE || eqv(name, prefix)) {
        return true;
    }

    if(name->objtype() != prefix->objtype()) {
        return false;
    }

    if(name->objtype() == CONS_TYPE) {
        shared_ptr<cons> name_cons = dynamic_pointer_cast<cons>(name);
        shared_ptr<cons> prefix_cons = dynamic_pointer_cast<cons>(prefix);
        return prefix_match(name_cons->car(), prefix_cons->car()) &&
            prefix_match(name_cons->cdr(), prefix_cons->cdr());
    }

    return false;
}

lexicalscope::lexicalscope() :
    parent(nullptr)
{

}

lexicalscope::lexicalscope(shared_ptr<lexicalscope> p) :
    parent(p)
{

}

void lexicalscope::defval(string name, shared_ptr<lispobj> value) {
    valbindings[name] = value;
}

void lexicalscope::defun(string name, shared_ptr<lispobj> value) {
    funbindings[name] = value;
}

void lexicalscope::undefval(string name) {
    valbindings.erase(name);
}

void lexicalscope::undefun(string name) {
    funbindings.erase(name);
}

void lexicalscope::setval(string name, shared_ptr<lispobj> value) {
    // try to find a variable binding. if it already exists,
    // set it in the same lexicalscope that we found it
    shared_ptr<lexicalscope> scope = shared_ptr<lexicalscope>(this);
    while(scope != nullptr) {
        auto iter = scope->valbindings.find(name);
        if(iter != scope->valbindings.end()) {
            iter->second = value;
            return;
        }
        scope = scope->parent;
    }

    // it has not been set yet, so set it in the current frame
    valbindings[name] = value;
}

void lexicalscope::setfun(string name, shared_ptr<lispobj> value) {
    // try to find a variable binding. if it already exists,
    // set it in the same lexicalscope that we found it
    cout << "Setting fun " << name << endl;
    shared_ptr<lexicalscope> scope = shared_ptr<lexicalscope>(this);
    while(scope != nullptr) {
        auto iter = scope->funbindings.find(name);
        if(iter != scope->funbindings.end()) {
            iter->second = value;
            return;
        }
        scope = scope->parent;
    }

    // it has not been set yet, so set it in the current frame
    funbindings[name] = value;
}

shared_ptr<lispobj> lexicalscope::getval(string name) {
    auto it = valbindings.find(name);
    if(it != valbindings.end()) {
        return it->second;
    } else {
        for(shared_ptr<module> mod : imports) {
            for(shared_ptr<symbol> sym : mod->get_exports()) {
                if(sym->name() == name) {
                    return mod->get_bindings()->getval(name);
                }
            }
        }

        if(parent) {
            return parent->getval(name);
        } else {
            //XXX: should be undefined so eval loop can error
            return make_shared<nil>();
        }
    }
}

shared_ptr<lispobj> lexicalscope::getfun(string name) {
    auto it = funbindings.find(name);
    if(it != funbindings.end()) {
        return it->second;
    } else {
        for(shared_ptr<module> mod : imports) {
            for(shared_ptr<symbol> sym : mod->get_exports()) {
                if(sym->name() == name) {
                    return mod->get_bindings()->getfun(name);
                }
            }
        }

        if(parent) {
            return parent->getfun(name);
        } else {
            //XXX: should be undefined so eval loop can error
            cout << "couldn't find fun " << name << endl;
            return make_shared<nil>();
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
    for(auto it = valbindings.begin(); it != valbindings.end(); ++it) {
        cout << it->first << ": ";
        it->second->print();
        cout<< endl;
    }

    for(auto it = funbindings.begin(); it != funbindings.end(); ++it) {
        cout << "(function " << it->first << "): ";
        it->second->print();
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
        frame.code->print();
        cout << endl;
        cout << "  args:" << endl;
        for(auto arg: frame.evaled_args) {
            cout << "    ";
            arg->print();
            cout << endl;
        }
    }
}

int apply_lispfunc(std::deque<stackframe>& exec_stack) {
    auto evaled_args = exec_stack.front().evaled_args;
    shared_ptr<lispfunc> func = dynamic_pointer_cast<lispfunc>(evaled_args.front());
    evaled_args.erase(evaled_args.begin());
    shared_ptr<lexicalscope> scope(new lexicalscope(func->closure));

    auto arg_value_iter = evaled_args.begin();
    shared_ptr<lispobj> arg_names = func->args;
    while(arg_value_iter != evaled_args.end() &&
          arg_names->objtype() == CONS_TYPE) {
        shared_ptr<cons> c = dynamic_pointer_cast<cons>(arg_names);
        shared_ptr<lispobj> name = c->car();
        if(name->objtype() == SYMBOL_TYPE) {
            shared_ptr<symbol> s = dynamic_pointer_cast<symbol>(name);
            scope->defval(s->name(), *arg_value_iter);
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
        func->args->print(); cout << endl;
        for(auto arg_value_iter = evaled_args.begin();
            arg_value_iter != evaled_args.end();
            ++arg_value_iter) {
            (*arg_value_iter)->print(); cout << " ";
        }
        cout << endl;
        arg_names->print(); cout << endl;
        return 1;
    }

    exec_stack.front().mark = applying;
    exec_stack.front().scope = scope;
    exec_stack.front().code = func->code;
    return 0;
}

bool is_special_form(shared_ptr<lispobj> form) {
    if(form->objtype() == SYMBOL_TYPE) {
        shared_ptr<symbol> sym = dynamic_pointer_cast<symbol>(form);
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
        } else if(sym->name() == "defun") {
            return true;
        } else if(sym->name() == "quote") { 
            return true;
        }
    }
    return false;
}

int add_import_to_module(shared_ptr<module> mod, shared_ptr<lispobj> importdecl) {
    if(importdecl->objtype() != CONS_TYPE) {
        return 1;
    }

    shared_ptr<cons> c = dynamic_pointer_cast<cons>(importdecl);
    if(c->cdr()->objtype() != NIL_TYPE) {
        cout << "too many arguments to import" << endl;
        return 1;
    }

//    mod->add_import(c->car());
    return 0;
}

int add_export_to_module(shared_ptr<module> mod, shared_ptr<lispobj> exportdecl) {
    if(exportdecl->objtype() != CONS_TYPE) {
        cout << "nothing to export" << endl;
        return 1;
    }

    while(exportdecl->objtype() == CONS_TYPE) {
        shared_ptr<cons> c = dynamic_pointer_cast<cons>(exportdecl);
        if(c->car()->objtype() != SYMBOL_TYPE) {
            cout << "non-symbol in export list" << endl;
            return 1;
        }

        mod->add_export(dynamic_pointer_cast<symbol>(c->car()));

        exportdecl = c->cdr();
    }

    return 0;
}

int add_defun_to_module(shared_ptr<module> mod,
                         shared_ptr<lispobj> definedecl,
                         shared_ptr<lexicalscope> scope) {
    if(definedecl->objtype() != CONS_TYPE) {
        return 1;
    }

    shared_ptr<cons> c = dynamic_pointer_cast<cons>(definedecl);
    if(c->car()->objtype() != SYMBOL_TYPE){
        return 1;
    }

    shared_ptr<symbol> defname = dynamic_pointer_cast<symbol>(c->car());

    if(c->cdr()->objtype() != CONS_TYPE) {
        return 1;
    }

    c = dynamic_pointer_cast<cons>(c->cdr());
    shared_ptr<lispobj> val = eval(c->car(), scope);
    if(val) {
        mod->defun(defname->name(), val);
    } else {
        cout << "error evaluating define for module" << endl;
        return 1;
    }

    return 0;
}

int eval_module_special_form(std::deque<stackframe>& exec_stack) {
    if(exec_stack.front().code->objtype() != CONS_TYPE) {
        cout << "module does not have name" << endl;
        return 1;
    }
    shared_ptr<lispobj> lobj;
    shared_ptr<cons> c = dynamic_pointer_cast<cons>(exec_stack.front().code);
    shared_ptr<module> m(new module(c->car()));

    lobj = c->cdr();
    while(lobj->objtype() == CONS_TYPE) {
        c = dynamic_pointer_cast<cons>(lobj);

        if(c->car()->objtype() != CONS_TYPE) {
            cout << "invalid module declaration:";
            c->print();
            cout << endl;
            return 1;
        }

        shared_ptr<cons> decl = dynamic_pointer_cast<cons>(c->car());

        if(decl->car()->objtype() != SYMBOL_TYPE) {
            cout << "invalid module declaration:";
            c->print();
            cout << endl;
            return 1;
        }

        shared_ptr<symbol> s = dynamic_pointer_cast<symbol>(decl->car());
        if(s->name() == "import") {
            if(add_import_to_module(m, decl->cdr())) {
                cout << "invalid import declaration: ";
                decl->print();
                cout << endl;
                return 1;
            }
        } else if(s->name() == "export") {
            if(add_export_to_module(m, decl->cdr())) {
                cout << "invalid export declaration: ";
                decl->print();
                cout << endl;
                return 1;
            }
        } else if(s->name() == "defun") {
            if(add_defun_to_module(m, decl->cdr(), exec_stack.front().scope)) {
                cout << "invalid define declaration: ";
                decl->print ();
                cout << endl;
                return 1;
            }
        } else if(s->name() == "init") {
            m->add_init(decl->cdr());
        }

        lobj = c->cdr();
    }

    //env->add_module_def(m);

    exec_stack.front().mark = evaled;
    exec_stack.front().code = m;

    return 0;
}

int eval_if_special_form(std::deque<stackframe>& exec_stack) {
    if(exec_stack.front().evaled_args.size() == 1) {
        shared_ptr<lispobj> lobj = exec_stack.front().code;
        if(lobj->objtype() != CONS_TYPE) {
            lobj->print();
            cout << " if has no condition" << endl;
            return 1;
        }
        shared_ptr<cons> c = dynamic_pointer_cast<cons>(lobj);
        exec_stack.front().code = c->cdr();
        exec_stack.push_front(stackframe(exec_stack.front().scope,
                                         evaluating,
                                         c->car()));
    } else if(exec_stack.front().evaled_args.size() == 2) {
        if(exec_stack.front().code->objtype() != CONS_TYPE) {
            cout << "if has no true branch" << endl;
            return 1;
        }
        shared_ptr<cons> c = dynamic_pointer_cast<cons>(exec_stack.front().code);

        if(exec_stack.front().evaled_args[1]->objtype() == NIL_TYPE) {
            if(c->cdr()->objtype() != CONS_TYPE) {
                exec_stack.front().mark = evaled;
                exec_stack.front().code = make_shared<nil>();
                exec_stack.front().evaled_args.clear();
            } else {
                shared_ptr<cons> c2 = dynamic_pointer_cast<cons>(c->cdr());
                exec_stack.front().mark = evaluating;
                exec_stack.front().code = c2->car();
                exec_stack.front().evaled_args.clear();
            }
        } else {
            exec_stack.front().mark = evaluating;
            exec_stack.front().code = c->car();
            exec_stack.front().evaled_args.clear();
        }
    }

    return 0;
}

int eval_defun_special_form(std::deque<stackframe>& exec_stack) {
    shared_ptr<lispobj> lobj = exec_stack.front().code;
    if(lobj->objtype() != CONS_TYPE) {
        cout << "define needs more arguments" << endl;
        return 1;
    }

    shared_ptr<cons> c = dynamic_pointer_cast<cons>(lobj);
    if(c->car()->objtype() != SYMBOL_TYPE) {
        cout << "defun: first argument must be symbol, instead got: ";
        c->car()->print();
        cout << endl;
        return 1;
    }

    shared_ptr<symbol> funcname = dynamic_pointer_cast<symbol>(c->car());

    if(c->cdr()->objtype() != CONS_TYPE) {
        cout << "defun: not enough arguments" << endl;
        return 1;
    }

    shared_ptr<cons> c2 = dynamic_pointer_cast<cons>(c->cdr());
    if(c2->car()->objtype() != CONS_TYPE &&
       c2->car()->objtype() != NIL_TYPE) {
        cout << "defun: second argument not list" << endl;
        return 1;
    }

    if(c2->cdr()->objtype() == NIL_TYPE) {
        cout << "defun: " << funcname->name() << " has no function body" << endl;
    }

    shared_ptr<lispfunc> lfunc(new lispfunc(c2->car(), exec_stack.front().scope, c2->cdr()));

    exec_stack.front().scope->defun(funcname->name(), lfunc);
    exec_stack.front().evaled_args.clear();
    exec_stack.front().mark = evaled;
    exec_stack.front().code = c->car();
    return 0;
}

int eval_lambda_special_form(std::deque<stackframe>& exec_stack) {
    shared_ptr<lispobj> lobj = exec_stack.front().code;
    if(lobj->objtype() != CONS_TYPE) {
        cout << "lambda needs more arguments" << endl;
        return 1;
    }

    shared_ptr<cons> c = dynamic_pointer_cast<cons>(lobj);
    if(c->cdr()->objtype() != CONS_TYPE) {
        cout << "lambda needs more arguments" << endl;
        return 1;
    }

    shared_ptr<cons> c2 = dynamic_pointer_cast<cons>(c->cdr());
    shared_ptr<lispfunc> lfunc(new lispfunc(c->car(), exec_stack.front().scope, c2));
    exec_stack.front().mark = evaled;
    exec_stack.front().code = lfunc;

    return 0;
}

int eval_special_form(string name,
                      std::deque<stackframe>& exec_stack) {
    if(name == "if") {
        return eval_if_special_form(exec_stack);
    } else if(name == "lambda") {
        return eval_lambda_special_form(exec_stack);
    } else if(name == "module") {
        return eval_module_special_form(exec_stack);
    } else if(name == "import") {
        shared_ptr<lispobj> lobj = exec_stack.front().code;
        if(lobj->objtype() != CONS_TYPE) {
            cout << "import needs more arguments" << endl;
            return 1;
        }

        shared_ptr<cons> c = dynamic_pointer_cast<cons>(lobj);
        shared_ptr<module> m = nullptr; //env->get_module_by_prefix(c->car());
        if(!m) {
            cout << "Module ";
            c->car()->print();
            cout << " not found." << endl;
            return 1;
        } else if(!(m->init())) {
            cout << "Module ";
            c->car()->print();
            cout << " init failed." << endl;
            return 1;
        }
        exec_stack.front().scope->add_import(m);
        exec_stack.front().mark = evaled;
        exec_stack.front().code = make_shared<nil>();
    } else if(name == "begin") {
        exec_stack.front().mark = applying;
    } else if(name == "defun") {
        return eval_defun_special_form(exec_stack);
    } else if(name == "quote") {
        shared_ptr<lispobj> lobj = exec_stack.front().code;
        if(lobj->objtype() != CONS_TYPE) {
            cout << "quote needs more arguments" << endl;
            return 1;
        }

        shared_ptr<cons> c = dynamic_pointer_cast<cons>(lobj);
        if(c->cdr()->objtype() != NIL_TYPE) {
            cout << "quote has too many args" << endl;
            return 1;
        }

        exec_stack.front().code = c->car();
        exec_stack.front().mark = evaled;
    }
    return 0;
}

shared_ptr<lispobj> eval(shared_ptr<lispobj> code,
                         shared_ptr<lexicalscope> tls) {
    std::deque<stackframe> exec_stack;
    shared_ptr<lispobj> nil_obj(new nil());

    exec_stack.push_front(stackframe(tls, evaluating, code));

    while(exec_stack.size() != 0 && (exec_stack.size() > 1 || exec_stack.front().mark != evaled)) {
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
                shared_ptr<cons> c = dynamic_pointer_cast<cons>(exec_stack.front().code);
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
                shared_ptr<cons> c = dynamic_pointer_cast<cons>(exec_stack.front().code);
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
                    if(apply_lispfunc(exec_stack) != 0) {
                        return nullptr;
                    }
                } else if(evaled_args.front()->objtype() == CFUNC_TYPE) {
                    shared_ptr<cfunc> func = dynamic_pointer_cast<cfunc>(evaled_args.front());
                    evaled_args.erase(evaled_args.begin());
                    shared_ptr<lispobj> ret = func->func(evaled_args);
                    if(!ret) {
                        return nullptr;
                    }
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
                shared_ptr<symbol> s = dynamic_pointer_cast<symbol>(exec_stack.front().code);

                if(s->name() == "nil") {
                    exec_stack.front().code = nil_obj;
                } else if(exec_stack.size() > 1 &&
                          exec_stack[1].mark == evaluating &&
                          exec_stack[1].evaled_args.size() == 0) {
                    // first element of a call, so look up a function
                    exec_stack.front().code = exec_stack.front().scope->getfun(s->name());
                } else {
                    exec_stack.front().code = exec_stack.front().scope->getval(s->name());
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
            shared_ptr<symbol> sym = dynamic_pointer_cast<symbol>(list_head);
            if(eval_special_form(sym->name(), exec_stack) != 0) {
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
        shared_ptr<number> n = dynamic_pointer_cast<number>(obj);
        sum += n->value();
    }
    return make_shared<number>(sum);
}

shared_ptr<lispobj> minus(vector<shared_ptr<lispobj> > args) {
    if(args.size() > 1) {
        if(args[0]->objtype() != NUMBER_TYPE) {
            cout << "minus requires numbers" << endl;
            return nullptr;
        }
        shared_ptr<number> n = dynamic_pointer_cast<number>(args[0]);
        int diff = n->value();
        args.erase(args.begin());

        for(auto obj : args) {
            if(obj->objtype() != NUMBER_TYPE) {
                cout << "minus requires numbers" << endl;
                return nullptr;
            }
            shared_ptr<number> n = dynamic_pointer_cast<number>(obj);
            diff -= n->value();
        }
        return make_shared<number>(diff);
    } else {
        shared_ptr<lispobj> obj(args[0]);
        if(obj->objtype() != NUMBER_TYPE) {
            cout << "minus requires numbers" << endl;
        }
        shared_ptr<number> n = dynamic_pointer_cast<number>(obj);
        return make_shared<number>(-(n->value()));
    }
}

shared_ptr<lispobj> multiply(vector<shared_ptr<lispobj> > args) {
    int product = 1;
    for(auto obj : args) {
        if(obj->objtype() != NUMBER_TYPE) {
            cout << "multiply requires numbers" << endl;
            return nullptr;
        }
        shared_ptr<number> n = dynamic_pointer_cast<number>(obj);
        product *= n->value();
    }
    return make_shared<number>(product);
}

shared_ptr<lispobj> divide(vector<shared_ptr<lispobj> > args) {
    if(args.size() > 1) {
        if(args[0]->objtype() != NUMBER_TYPE) {
            cout << "divide requires numbers" << endl;
            return nullptr;
        }
        shared_ptr<number> n = dynamic_pointer_cast<number>(args[0]);
        int quotient = n->value();
        args.erase(args.begin());

        for(auto obj : args) {
            if(obj->objtype() != NUMBER_TYPE) {
                cout << "divide requires numbers" << endl;
                return nullptr;
            }
            shared_ptr<number> n = dynamic_pointer_cast<number>(obj);
            quotient /= n->value();
        }
        return make_shared<number>(quotient);
    } else {
        shared_ptr<lispobj> obj(args[0]);
        if(obj->objtype() != NUMBER_TYPE) {
            cout << "divide requires numbers" << endl;
        }
        shared_ptr<number> n = dynamic_pointer_cast<number>(obj);
        return make_shared<number>(n->value());
    }
}

shared_ptr<lispobj> print_cfunc(vector< shared_ptr<lispobj> > args) {
    for(shared_ptr<lispobj> lobj : args) {
        lobj->print();
    }

    return make_shared<nil>();
}

shared_ptr<lispobj> newline(vector< shared_ptr<lispobj> > /*args*/) {
    cout << endl;
    return make_shared<nil>();
}

shared_ptr<lispobj> list_cfunc(vector< shared_ptr<lispobj> > args) {
    return make_reverse_list(args.rbegin(), args.rend());
}

shared_ptr<lispobj> eq_cfunc(vector< shared_ptr<lispobj> > args) {
    if(args.size() != 2) {
        cout << "ERROR eq wants 2 args" << endl;
        return nullptr;
    }

    if(eq(args[0], args[1])) {
        return make_shared<symbol>("t");
    } else {
        return make_shared<nil>();
    }
}

shared_ptr<lispobj> eqv_cfunc(vector< shared_ptr<lispobj> > args) {
    if(args.size() != 2) {
        cout << "ERROR eqv wants 2 args" << endl;
        return nullptr;
    }

    if(eqv(args[0], args[1])) {
        return make_shared<symbol>("t");
    } else {
        return make_shared<nil>();
    }
}

shared_ptr<lispobj> equal_cfunc(vector< shared_ptr<lispobj> > args) {
    if(args.size() != 2) {
        cout << "ERROR equal wants 2 args" << endl;
        return nullptr;
    }

    if(equal(args[0], args[1])) {
        return make_shared<symbol>("t");
    } else {
        return make_shared<nil>();
    }
}

shared_ptr<lispobj> cons_cfunc(vector< shared_ptr<lispobj> > args) {
    if(args.size() != 2) {
        cout << "ERROR cons wants 2 args" << endl;
        return nullptr;
    }

    return make_shared<cons>(args[0], args[1]);
}

shared_ptr<lispobj> append_cfunc(vector< shared_ptr<lispobj> > args) {
    shared_ptr<lispstring> lstr(new lispstring());

    for(auto it = args.begin(); it != args.end(); ++it) {
        shared_ptr<lispstring> arg = dynamic_pointer_cast<lispstring>(*it);
        if(!arg) {
            cout << "ERROR append wants only strings" << endl;
            return nullptr;
        }

        lstr->append(arg);
    }

    return lstr;
}

shared_ptr<lispobj> open_file_cfunc(vector< shared_ptr<lispobj> > args) {
    if(args.size() != 1 && !dynamic_pointer_cast<lispstring>(args[0])) {
        cout << "ERROR open-file wants one string" << endl;
        return nullptr;
    }

    shared_ptr<lispstring> filename = dynamic_pointer_cast<lispstring>(args[0]);
    return make_shared<fileinputport>(filename->get_contents());
}

shared_ptr<lispobj> read_cfunc(vector< shared_ptr<lispobj> > args) {
    if(args.size() != 1 && !dynamic_pointer_cast<fileinputport>(args[0])) {
        cout << "ERROR read wants on input-port" << endl;
        return nullptr;
    }

    shared_ptr<fileinputport> port = dynamic_pointer_cast<fileinputport>(args[0]);
    return port->read();
}

shared_ptr<module> make_builtins_module() {
    shared_ptr<lispobj> module_name(new cons(make_shared<symbol>("builtins"), make_shared<nil>()));
    shared_ptr<module> builtins_module(new module(module_name));
    builtins_module->defun_and_export("+", make_shared<cfunc>(plus));
    builtins_module->defun_and_export("-", make_shared<cfunc>(minus));
    builtins_module->defun_and_export("*", make_shared<cfunc>(multiply));
    builtins_module->defun_and_export("/", make_shared<cfunc>(divide));
    builtins_module->defun_and_export("print", make_shared<cfunc>(print_cfunc));
    builtins_module->defun_and_export("newline", make_shared<cfunc>(newline));
    builtins_module->defun_and_export("list", make_shared<cfunc>(list_cfunc));
    builtins_module->defun_and_export("eq", make_shared<cfunc>(eq_cfunc));
    builtins_module->defun_and_export("eqv", make_shared<cfunc>(eqv_cfunc));
    builtins_module->defun_and_export("equal", make_shared<cfunc>(equal_cfunc));
    builtins_module->defun_and_export("cons", make_shared<cfunc>(cons_cfunc));
    builtins_module->defun_and_export("append", make_shared<cfunc>(append_cfunc));
    builtins_module->defun_and_export("open-file", make_shared<cfunc>(open_file_cfunc));
    builtins_module->defun_and_export("read", make_shared<cfunc>(read_cfunc));

    return builtins_module;
}
