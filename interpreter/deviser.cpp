#include <istream>
#include <iostream>
#include <sstream>
#include <stack>
#include <typeinfo>

#include "deviser.hpp"

using std::cout;
using std::endl;
using std::dynamic_pointer_cast;
using std::make_shared;
using std::stringstream;

lispobj::lispobj() {}
lispobj::~lispobj() {}

nil::nil() {}

void nil::print(ostream& out) {
    out << "'()";
}

symbol::symbol(string sn) {
    symname = sn;
}

string symbol::name() const {
    return symname;
}

void symbol::print(ostream& out) {
    out << symname;
}

cons::cons(shared_ptr<lispobj> a, shared_ptr<lispobj> d) {
    first = a;
    second = d;
}

shared_ptr<lispobj> cons::car() const {
    return first;
}

shared_ptr<lispobj> cons::cdr() const {
    return second;
}

void cons::set_car(shared_ptr<lispobj> a) {
    first = a;
}

void cons::set_cdr(shared_ptr<lispobj> d) {
    second = d;
}

void cons::print(ostream& out) {
    out << '(';
    car()->print(out);
    shared_ptr<cons> c;
    shared_ptr<lispobj> obj = cdr();
    while((c = dynamic_pointer_cast<cons>(obj))) {
        out << ' ';
        c->car()->print(out);
        obj = c->cdr();
    }

    if(dynamic_pointer_cast<nil>(obj)) {
        out << ')';
    } else {
        out << " . ";
        obj->print(out);
        out << ')';
    }
}

number::number(int n) {
    num = n;
}

int number::value() const {
    return num;
}

void number::print(ostream& out) {
    out << value();
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

void lispstring::print(ostream& out) {
    out << contents;
}

void lispstring::repr(ostream& out) {
    out << "\"";
    for(auto it = contents.begin(); it != contents.end(); ++it) {
        switch(*it) {
        case '"':
            out << "\\\"";
            break;
        case '\\':
            out << "\\\\";
            break;
        case '\n':
            out << "\\n";
            break;
        case '\t':
            out << "\\t";
            break;
        default:
            out << *it;
            break;
        }
    }
    out << "\"";
}

lispfunc::lispfunc(shared_ptr<lispobj> _args,
                   shared_ptr<lexicalscope> _closure,
                   shared_ptr<lispobj> _code) :
    args(_args),
    closure(_closure),
    code(_code)
{

}

void lispfunc::print(ostream& out) {
    make_shared<cons>(make_shared<symbol>("lambda"),
                      make_shared<cons>(args, code))->print(out);
}

macro::macro(shared_ptr<lispobj> _args,
             shared_ptr<lexicalscope> _closure,
             shared_ptr<lispobj> _code) :
    args(_args),
    closure(_closure),
    code(_code)
{

}

void macro::print(ostream& out) {
    make_shared<cons>(make_shared<symbol>("macro"),
                      make_shared<cons>(args, code))->print(out);
}

cfunc::cfunc(std::function<shared_ptr<lispobj>(vector<shared_ptr<lispobj> >)> f) :
    func(f)
{

}

void cfunc::print(ostream& out) {
    out << "CFUNC";
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
        return make_shared<lispstring>("");
    }
}

shared_ptr<lispobj> fileinputport::readchar() {
    char buf;
    instream.get(buf);
    if(instream) {
        return make_shared<lispstring>(string(&buf, 1));
    } else {
        return make_shared<lispstring>("");
    }
}

void fileinputport::print(ostream& out) {
    out << "<fileinputport: " << filename << ">";
}


module::module(shared_ptr<lispobj> _name, shared_ptr<lexicalscope> enc_scope) :
    name(_name),
    module_scope(new lexicalscope(enc_scope)),
    inited(false)
{
    module_scope->set_ismodulescope(true);
}

string get_command_name(shared_ptr<lispobj> command) {
    shared_ptr<cons> c;
    if(!(c = dynamic_pointer_cast<cons>(command))) {
        return "";
    }

    shared_ptr<symbol> sym;
    if(!(sym = dynamic_pointer_cast<symbol>(c->car()))) {
        return "";
    }

    return sym->name();
}

shared_ptr<lispobj> module::eval(shared_ptr<lispobj> command) {
    string command_name = get_command_name(command);

    if(command_name == "defun" || command_name == "defmacro") {
        defines.push_back(command);
        return ::eval(command, module_scope);
    } else if(command_name == "undefine") {

    } else if(command_name == "export") {
        shared_ptr<cons> command_cons = dynamic_pointer_cast<cons>(command);
        shared_ptr<lispobj> exportdecl = command_cons->cdr();

        shared_ptr<cons> c = dynamic_pointer_cast<cons>(exportdecl);
        if(!c) {
            cout << "nothing to export" << endl;
            return make_shared<nil>();
        }

        while(c) {
            shared_ptr<symbol> exportname = dynamic_pointer_cast<symbol>(c->car());
            if(!exportname) {
                cout << "non-symbol in export list" << endl;
                return make_shared<nil>();
            }

            add_export(exportname);

            c = dynamic_pointer_cast<cons>(c->cdr());
        }
        return command_cons->cdr();
    } else if(command_name == "unexport") {

    } else if(command_name == "import") {
        shared_ptr<cons> command_cons = dynamic_pointer_cast<cons>(command);
        shared_ptr<lispobj> importdecl = command_cons->cdr();

        shared_ptr<cons> c = dynamic_pointer_cast<cons>(importdecl);
        if(!c) {
            return make_shared<nil>();
        }

        if(!dynamic_pointer_cast<nil>(c->cdr())) {
            cout << "too many arguments to import" << endl;
            return make_shared<nil>();
        }

        shared_ptr<module> module_for_import(module_scope->find_module(c->car()));
        if(module_for_import) {
            add_import(module_for_import);
            return c->car();
        } else {
            cout << "Couldn't find module: ";
            c->car()->print();
            cout << endl;
        }
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
        return ::eval(command, module_scope);
    }

    return make_shared<nil>();
}

void module::add_import(shared_ptr<module> mod) {
    imports.push_back(mod->get_name());
    module_scope->add_import(mod);
}

void module::add_export(shared_ptr<symbol> sym) {
    exports.push_back(sym);
}

void module::defval(string name, shared_ptr<lispobj> value) {
    module_scope->defval(name, value);
}

void module::defun(string name, shared_ptr<lispobj> value) {
    module_scope->defun(name, value);
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
    return module_scope;
}

bool module::init() {
    if(inited) {
        return true;;
    }

    for(auto imp : imports) {
        shared_ptr<module> mod = nullptr; //env->get_module_by_prefix(imp);
        mod->init();
        module_scope->add_import(mod);
    }

    // it would be good if this could go on the stack of the currently running eval
    // but that's ~*~hard~*~, and this is easy
    for(auto initblock : initblocks) {
        shared_ptr<lexicalscope> initscope(new lexicalscope(module_scope));
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

void module::print(ostream& out) {
    out << "(module ";
    get_name()->print(out);
    out << " (imports ";
    auto imports = get_imports();
    for(auto import : imports) {
        import->print(out);
        out << " ";
    }
    out << ") (exports ";
    auto exports = get_exports();
    for(auto exprt : exports) {
        exprt->print(out);
        out << " ";
    }
    out << ") (init ";
    auto inits = get_initblocks();
    for(auto init : inits) {
        init->print(out);
        out  << " ";
    }
    out << ") ";
    get_bindings()->dump();
    out << ")";
}

syntaxlocation::syntaxlocation(string name, int linenum, int charnum) {
    this->name = name;
    this->linenum = linenum;
    this->charnum = charnum;
}

syntax::syntax(shared_ptr<syntaxlocation> loc, shared_ptr<syntax> par) :
    location(loc),
    parent(par)
{
}

syntax::~syntax() {
}

shared_ptr<syntaxlocation> syntax::get_location() {
    return location;
}

shared_ptr<syntax> syntax::get_parent() {
    return parent;
}

void syntax::set_parent(shared_ptr<syntax> p) {
    parent = p;
}

syntaxnil::syntaxnil(shared_ptr<syntaxlocation> loc, shared_ptr<syntax> par) :
    syntax(loc, par)
{
}

syntaxsymbol::syntaxsymbol(string sn, shared_ptr<syntaxlocation> loc, shared_ptr<syntax> par) :
    syntax(loc, par),
    symbol(sn)
{
}

syntaxcons::syntaxcons(shared_ptr<lispobj> a,
                       shared_ptr<lispobj> d,
                       shared_ptr<syntaxlocation> loc,
                       shared_ptr<syntax> par) :
    syntax(loc, par),
    cons(a, d)
{
}

syntaxnumber::syntaxnumber(int num, shared_ptr<syntaxlocation> loc, shared_ptr<syntax> par) :
    syntax(loc, par),
    number(num)
{
}

syntaxstring::syntaxstring(const string& str, shared_ptr<syntaxlocation> loc, shared_ptr<syntax> par) :
    syntax(loc, par),
    lispstring(str)
{
}

bool eq(shared_ptr<lispobj> left, shared_ptr<lispobj> right) {
    if(left == right) return true;

    if(dynamic_pointer_cast<symbol>(left) &&
       dynamic_pointer_cast<symbol>(right)) {
        shared_ptr<symbol> ls(dynamic_pointer_cast<symbol>(left));
        shared_ptr<symbol> rs(dynamic_pointer_cast<symbol>(right));
        return ls->name() == rs->name();
    } else if(dynamic_pointer_cast<nil>(left) &&
              dynamic_pointer_cast<nil>(right)) {
        return true;
    } else {
        return false;
    }
}

bool eqv(shared_ptr<lispobj> left, shared_ptr<lispobj> right) {
    if(eq(left, right)) return true;

    if(dynamic_pointer_cast<number>(left) &&
       dynamic_pointer_cast<number>(right)) {
        shared_ptr<number> ln(dynamic_pointer_cast<number>(left));
        shared_ptr<number> rn(dynamic_pointer_cast<number>(right));
        return ln->value() == rn->value();
    } else {
        return false;
    };
}

bool equal(shared_ptr<lispobj> left, shared_ptr<lispobj> right) {
    if(eqv(left, right)) return true;

    if(dynamic_pointer_cast<cons>(left) &&
       dynamic_pointer_cast<cons>(right)) {
        shared_ptr<cons> lc(dynamic_pointer_cast<cons>(left));
        shared_ptr<cons> rc(dynamic_pointer_cast<cons>(right));
        return equal(lc->car(), rc->car()) && equal(lc->cdr(), rc->cdr());
    } else if(dynamic_pointer_cast<lispstring>(left) &&
              dynamic_pointer_cast<lispstring>(right)) {
        shared_ptr<lispstring> left_string(dynamic_pointer_cast<lispstring>(left));
        shared_ptr<lispstring> right_string(dynamic_pointer_cast<lispstring>(right));
        return left_string->get_contents() == right_string->get_contents();
    } else {
        return false;
    };
}

void printall(vector< shared_ptr<lispobj> > objs) {
    for(auto obj : objs) {
        obj->print();
        cout << endl;
    }
}

reader::reader(shared_ptr<std::istream> in, string name) :
    input(in),
    streamname(name),
    linenum(1),
    colnum(1)
{
}

shared_ptr<lispobj> reader::read(shared_ptr<syntax> parent) {
    // remove leading whitespace

    bool iscomment = false;

    do {
        iscomment = false;

        while(isspace(peek_char())) {
            get_char();
        }

        if(peek_char() == ';') {
            iscomment = true;

            char c = get_char();
            while(c != '\n') {
                c = get_char();
                cout.flush();
            }
        }
    } while(iscomment);

    int line = linenum;
    int col = colnum;

    if(peek_char() == ')' || peek_char() == std::char_traits<char>::eof()) {
        return nullptr;
    }

    // figure out type of object
    if(peek_char() == '(') { //list
        get_char();
        shared_ptr<lispobj> listtoreturn = make_shared<syntaxnil>(make_shared<syntaxlocation>(streamname, line, col), parent);
        shared_ptr<cons> placeinlist(nullptr);

        while(peek_char() != ')') {
            while(isspace(peek_char())) {
                get_char();
            }

            auto obj = read(dynamic_pointer_cast<syntax>(listtoreturn));
            if(obj) {
                if(dynamic_pointer_cast<nil>(listtoreturn)) {
                    listtoreturn = make_shared<syntaxcons>(obj, listtoreturn, make_shared<syntaxlocation>(streamname, line, col), parent);
                    shared_ptr<syntax> syntaxobj = dynamic_pointer_cast<syntax>(obj);
                    syntaxobj->set_parent(dynamic_pointer_cast<syntax>(listtoreturn));
                    placeinlist = dynamic_pointer_cast<cons>(listtoreturn);
                } else {
                    placeinlist->set_cdr(make_shared<syntaxcons>(obj, placeinlist->cdr(), make_shared<syntaxlocation>(streamname, line, col), parent));
                    placeinlist = dynamic_pointer_cast<cons>(placeinlist->cdr());
                }
            }
        }

        //update location of the nil to the end of the list
        if(placeinlist) {
            shared_ptr<syntax> syn = dynamic_pointer_cast<syntax>(placeinlist->cdr());
            syn->get_location()->charnum = colnum;
            syn->get_location()->linenum = linenum;
        }

        get_char();
        return listtoreturn;
    } else if(isdigit(peek_char())) { // number ('.' too, once we have non-integers)
        string numstring;

        while(isdigit(peek_char())) {
            numstring.push_back(get_char());
        }

        size_t pos = 0;
        int n = std::stoi(numstring, &pos);
        return make_shared<syntaxnumber>(n, make_shared<syntaxlocation>(streamname, line, col), parent);
    } else if(peek_char() == '"') {
        get_char();

        string contents;

        while(peek_char() != '"') {
            char next = get_char();

            if(next == '\\') {
                char escapedchar = get_char();

                if(escapedchar == 'n') {
                    contents.push_back('\n');
                } else {
                    contents.push_back(escapedchar);
                }
            } else {
                contents.push_back(next);
            }
        }
        get_char();

        return make_shared<syntaxstring>(contents, make_shared<syntaxlocation>(streamname, line, col), parent);
    } else { //symbol
        string sym_name;
        char next = peek_char();

        while(!isspace(next) &&
              next != '(' &&
              next != ')' &&
              next != ';' &&
              next != std::char_traits<char>::eof()) {
            sym_name.push_back(get_char());
            next = peek_char();
        }

        return make_shared<syntaxsymbol>(sym_name, make_shared<syntaxlocation>(streamname, line, col), parent);
    }
}

vector< shared_ptr<lispobj> > reader::readall() {
    vector< shared_ptr<lispobj> > ret;

    while(input->good()) {
        shared_ptr<lispobj> lobj = read();
        if(lobj) {
            ret.push_back(lobj);
        }
    }

    return ret;
}

char reader::get_char() {
    char ret = input->get();
    if(ret == '\n') {
        ++linenum;
        colnum = 1;
    } else {
        ++colnum;
    }

    return ret;
}

char reader::peek_char() {
    return input->peek();
}

shared_ptr<lispobj> read(string str) {
    shared_ptr<std::stringstream> sstream(new std::stringstream(str));
    reader r(std::static_pointer_cast<std::istream>(sstream), "INPUT");
    return r.read();
}

vector< shared_ptr<lispobj> > readall(string str) {
    shared_ptr<std::stringstream> sstream(new std::stringstream(str));
    reader r(std::static_pointer_cast<std::istream>(sstream), "INPUT");

    return r.readall();
}

bool prefix_match(shared_ptr<lispobj> name, shared_ptr<lispobj> prefix) {
    if(dynamic_pointer_cast<nil>(prefix) || eqv(name, prefix)) {
        return true;
    }

    shared_ptr<cons> name_cons = dynamic_pointer_cast<cons>(name);
    shared_ptr<cons> prefix_cons = dynamic_pointer_cast<cons>(prefix);

    if(name_cons && prefix_cons) {
        return prefix_match(name_cons->car(), prefix_cons->car()) &&
            prefix_match(name_cons->cdr(), prefix_cons->cdr());
    }

    return false;
}

lexicalscope::lexicalscope() :
    parent(nullptr),
    ismodulescope(false)
{

}

lexicalscope::lexicalscope(shared_ptr<lexicalscope> p) :
    parent(p),
    ismodulescope(false)
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

        if(ismodulescope) {
            scope = NULL;
        } else {
            scope = scope->parent;
        }
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

void lexicalscope::set_ismodulescope(bool ismodulescope) {
    this->ismodulescope = ismodulescope;
}

shared_ptr<lispobj> find_val_in_module(shared_ptr<module> mod, string name) {
    for(shared_ptr<symbol> sym : mod->get_exports()) {
        if(sym->name() == name) {
            return mod->get_bindings()->getval(name);
        }
    }

    return nullptr;
}

shared_ptr<lispobj> lexicalscope::getval(string name) {
    auto it = valbindings.find(name);
    if(it != valbindings.end()) {
        return it->second;
    } else {
        for(shared_ptr<module> mod : imports) {
            shared_ptr<lispobj> ret = find_val_in_module(mod, name);

            if(ret) {
                return ret;
            }
        }

        if(parent && !ismodulescope) {
            return parent->getval(name);
        } else {
            //XXX: should be undefined so eval loop can error
            return make_shared<nil>();
        }
    }
}

shared_ptr<lispobj> find_fun_in_module(shared_ptr<module> mod, string name) {
    for(shared_ptr<symbol> sym : mod->get_exports()) {
        if(sym->name() == name) {
            return mod->get_bindings()->getfun(name);
        }
    }

    return nullptr;
}

shared_ptr<lispobj> lexicalscope::getfun(string name) {
    auto it = funbindings.find(name);
    if(it != funbindings.end()) {
        return it->second;
    } else {
        for(shared_ptr<module> mod : imports) {
            shared_ptr<lispobj> ret = find_fun_in_module(mod, name);

            if(ret) {
                return ret;
            }
        }

        if(parent && !ismodulescope) {
            return parent->getfun(name);
        } else {
            //XXX: should be undefined so eval loop can error
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

shared_ptr<module> lexicalscope::find_module(shared_ptr<lispobj> module_prefix) {
    for(auto module : imports) {
        if(prefix_match(module->get_name(), module_prefix)) {
            return module;
        }
    }

    if(parent) {
        return parent->find_module(module_prefix);
    }

    return nullptr;
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
const int evalmacro = 4;

void print_stack(const std::deque<stackframe> exec_stack) {
    cout << "Stack size: " << exec_stack.size() << endl;

    for(auto frame : exec_stack) {
        cout << "frame " << frame.mark << endl;
        cout << "  code: ";
        frame.code->print();
        cout << endl;
        shared_ptr<syntax> code_syntax = dynamic_pointer_cast<syntax>(frame.code);
        if(code_syntax) {
            shared_ptr<syntaxlocation> loc = code_syntax->get_location();
            cout << "  location: " << loc->name << " (" << loc->linenum << ", " << loc->charnum
                 << ")" << endl;
        } else {
            cout << "  no code location info" << endl;
        }
        cout << "  args:" << endl;
        for(auto arg: frame.evaled_args) {
            cout << "    ";
            arg->print();
            cout << endl;
        }
    }
}

void apply_lispfunc(std::deque<stackframe>& exec_stack) {
    auto evaled_args = exec_stack.front().evaled_args;
    shared_ptr<lispfunc> func = dynamic_pointer_cast<lispfunc>(evaled_args.front());
    //cout << "applying "; func->print(); cout << endl;
    evaled_args.erase(evaled_args.begin());
    shared_ptr<lexicalscope> scope(new lexicalscope(func->closure));

    auto arg_value_iter = evaled_args.begin();
    shared_ptr<lispobj> arg_names = func->args;
    shared_ptr<cons> c = dynamic_pointer_cast<cons>(arg_names);
    while(arg_value_iter != evaled_args.end() && c) {
        shared_ptr<symbol> name = dynamic_pointer_cast<symbol>(c->car());
        if(name) {
            if(name->name() == "&rest") {
                arg_names = c->cdr();
                c = dynamic_pointer_cast<cons>(arg_names);
                if(c){
                    name = dynamic_pointer_cast<symbol>(c->car());
                    if(name) {
                        scope->defval(name->name(), make_list(arg_value_iter, evaled_args.end()));
                        arg_value_iter = evaled_args.end();
                        arg_names = c->cdr();
                        c = dynamic_pointer_cast<cons>(arg_names);
                        continue;
                    } else {
                        throw string("ERROR: arguments must be symbols");
                    }
                } else {
                    throw("ERROR: need an argument after &rest");
                }
            } else {
                scope->defval(name->name(), *arg_value_iter);
            }
        } else {
            throw string("ERROR: arguments must be symbols");
        }
        arg_names = c->cdr();
        c = dynamic_pointer_cast<cons>(arg_names);
        ++arg_value_iter;
    }

    if(!dynamic_pointer_cast<nil>(arg_names) ||
       arg_value_iter != evaled_args.end()) {
        throw string("ERROR: function arity does not match call.");
    }

    exec_stack.front().mark = applying;
    exec_stack.front().scope = scope;
    exec_stack.front().code = func->code;
}

void apply_macro(std::deque<stackframe>& exec_stack) {
    auto evaled_args = exec_stack.front().evaled_args;
    shared_ptr<macro> func = dynamic_pointer_cast<macro>(evaled_args.front());
    //cout << "applying "; func->print(); cout << endl;

    shared_ptr<lexicalscope> scope(new lexicalscope(func->closure));

    shared_ptr<lispobj> args = exec_stack.front().code;
    shared_ptr<cons> args_cons = dynamic_pointer_cast<cons>(args);
    shared_ptr<lispobj> param_names = func->args;
    shared_ptr<cons> param_cons = dynamic_pointer_cast<cons>(param_names);
    while(args_cons && param_cons) {
        shared_ptr<symbol> param_name = dynamic_pointer_cast<symbol>(param_cons->car());
        if(param_name) {
            if(param_name->name() == "&rest") {
                param_names = param_cons->cdr();
                param_cons = dynamic_pointer_cast<cons>(param_names);
                args = make_shared<cons>(args, make_shared<nil>());
                args_cons = dynamic_pointer_cast<cons>(args);
                continue;
            } else {
                scope->defval(param_name->name(), args_cons->car());
            }
        } else {
            throw string("ERROR: parameter names must be symbols");
        }
        param_names = param_cons->cdr();
        param_cons = dynamic_pointer_cast<cons>(param_names);
        args = args_cons->cdr();
        args_cons = dynamic_pointer_cast<cons>(args);
    }

    // handle rest param == nil
    if(param_cons) {
        shared_ptr<symbol> param_name = dynamic_pointer_cast<symbol>(param_cons->car());
        if(param_name && param_name->name() == "&rest") {
            param_names = param_cons->cdr();
            param_cons = dynamic_pointer_cast<cons>(param_names);
            if(param_cons) {
                param_name = dynamic_pointer_cast<symbol>(param_cons->car());
                if(param_name) {
                    param_names = param_cons->cdr();
                    scope->defval(param_name->name(), make_shared<nil>());
                }
            }
        }
    }

    if(!dynamic_pointer_cast<nil>(param_names) ||
       !dynamic_pointer_cast<nil>(args)) {
        stringstream ss;
        ss << "ERROR: macro arity does not match call." << endl;
        ss << "Remaining param_names: "; param_names->print(ss); ss << endl;
        ss << "Remaining args: "; args->print(ss);
        throw ss.str();
    }

    exec_stack.front().mark = evalmacro;
    exec_stack.push_front(stackframe(scope, applying, func->code));
}

bool is_special_form(shared_ptr<lispobj> form) {
    shared_ptr<symbol> sym = dynamic_pointer_cast<symbol>(form);
    if(sym) {
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
        } else if(sym->name() == "defmacro") {
            return true;
        } else if(sym->name() == "quote") { 
            return true;
        } else if(sym->name() == "let*") {
            return true;
        }
    }
    return false;
}

int add_import_to_module(shared_ptr<module> mod, shared_ptr<lispobj> importdecl) {
    shared_ptr<cons> c = dynamic_pointer_cast<cons>(importdecl);
    if(!c) {
        return 1;
    }

    if(!dynamic_pointer_cast<nil>(c->cdr())) {
        cout << "too many arguments to import" << endl;
        return 1;
    }

    shared_ptr<module> module_for_import = mod->get_bindings()->find_module(c->car());
    if(module_for_import) {
        mod->add_import(module_for_import);
        return 0;
    } else {
        cout << "Cannot find module named: ";
        c->car()->print();
        cout << endl;
        return 1;
    }
}

int add_export_to_module(shared_ptr<module> mod, shared_ptr<lispobj> exportdecl) {
    shared_ptr<cons> c = dynamic_pointer_cast<cons>(exportdecl);
    if(!c) {
        cout << "nothing to export" << endl;
        return 1;
    }

    while(c) {
        shared_ptr<symbol> exportname = dynamic_pointer_cast<symbol>(c->car());
        if(!exportname) {
            cout << "non-symbol in export list" << endl;
            return 1;
        }

        mod->add_export(exportname);

        c = dynamic_pointer_cast<cons>(c->cdr());
    }

    return 0;
}

int add_defun_to_module(shared_ptr<module> mod,
                        shared_ptr<lispobj> defundecl) {
    shared_ptr<cons> c = dynamic_pointer_cast<cons>(defundecl);
    if(!c) {
        return 1;
    }

    shared_ptr<symbol> defname = dynamic_pointer_cast<symbol>(c->car());

    if(!defname) {
        return 1;
    }

    c = dynamic_pointer_cast<cons>(c->cdr());

    if(!c) {
        return 1;
    }

    shared_ptr<lispobj> arg_list = c->car();
    shared_ptr<lispobj> code = c->cdr();
    shared_ptr<lispfunc> lfunc(new lispfunc(arg_list, mod->get_bindings(), code));
    mod->defun(defname->name(), lfunc);

    return 0;
}

int add_defmacro_to_module(shared_ptr<module> mod,
                           shared_ptr<lispobj> defundecl) {
    shared_ptr<cons> c = dynamic_pointer_cast<cons>(defundecl);
    if(!c) {
        return 1;
    }

    shared_ptr<symbol> defname = dynamic_pointer_cast<symbol>(c->car());

    if(!defname) {
        return 1;
    }

    c = dynamic_pointer_cast<cons>(c->cdr());

    if(!c) {
        return 1;
    }

    shared_ptr<lispobj> arg_list = c->car();
    shared_ptr<lispobj> code = c->cdr();
    shared_ptr<macro> mac(new macro(arg_list, mod->get_bindings(), code));
    mod->defun(defname->name(), mac);

    return 0;
}

void eval_module_special_form(std::deque<stackframe>& exec_stack) {
    shared_ptr<cons> c = dynamic_pointer_cast<cons>(exec_stack.front().code);
    if(!c) {
        throw string("module does not have name");
    }

    shared_ptr<lispobj> lobj;
    shared_ptr<module> m(new module(c->car(),
                                    exec_stack.front().scope));

    c = dynamic_pointer_cast<cons>(c->cdr());
    while(c) {
        shared_ptr<cons> decl = dynamic_pointer_cast<cons>(c->car());
        if(!decl) {
            stringstream errormsg;
            errormsg << "invalid module declaration 1: ";
            c->car()->print(errormsg);
            errormsg << endl;
            throw errormsg.str();
        }


        shared_ptr<symbol> s = dynamic_pointer_cast<symbol>(decl->car());
        if(!s) {
            stringstream errormsg;
            errormsg << "invalid module declaration 2: ";
            c->print(errormsg);
            errormsg << endl;
            throw errormsg.str();
        }

        if(s->name() == "import") {
            if(add_import_to_module(m, decl->cdr())) {
                stringstream errormsg;
                errormsg << "invalid import declaration: ";
                decl->print(errormsg);
                errormsg << endl;
                throw errormsg.str();
            }
        } else if(s->name() == "export") {
            if(add_export_to_module(m, decl->cdr())) {
                stringstream errormsg;
                errormsg << "invalid export declaration: ";
                decl->print(errormsg);
                errormsg << endl;
                throw errormsg.str();
            }
        } else if(s->name() == "defun") {
            if(add_defun_to_module(m, decl->cdr())) {
                stringstream errormsg;
                errormsg << "invalid defun declaration: ";
                decl->print(errormsg);
                errormsg << endl;
                throw errormsg.str();
            }
        } else if(s->name() == "defmacro") {
            if(add_defmacro_to_module(m, decl->cdr())) {
                stringstream errormsg;
                errormsg << "invalid defmacro declaration: ";
                decl->print(errormsg);
                errormsg << endl;
                throw errormsg.str();
            }
        } else if(s->name() == "init") {
            m->add_init(decl->cdr());
        }

        c = dynamic_pointer_cast<cons>(c->cdr());
    }

    exec_stack.front().scope->add_import(m);

    exec_stack.front().mark = evaled;
    exec_stack.front().code = m;
}

void eval_if_special_form(std::deque<stackframe>& exec_stack) {
    if(exec_stack.front().evaled_args.size() == 1) {
        shared_ptr<lispobj> lobj = exec_stack.front().code;
        shared_ptr<cons> c = dynamic_pointer_cast<cons>(lobj);
        if(!c) {
            std::stringstream errormsg;
            lobj->print(errormsg);
            errormsg << " if has no condition" << endl;
            throw errormsg.str();
        }
        exec_stack.front().code = c->cdr();
        exec_stack.push_front(stackframe(exec_stack.front().scope,
                                         evaluating,
                                         c->car()));
    } else if(exec_stack.front().evaled_args.size() == 2) {
        shared_ptr<cons> c = dynamic_pointer_cast<cons>(exec_stack.front().code);
        if(!c) {
            throw string("if has no true branch");
        }

        if(!istrue(exec_stack.front().evaled_args[1])) {
            shared_ptr<cons> c2 = dynamic_pointer_cast<cons>(c->cdr());
            if(c2) {
                exec_stack.front().mark = evaluating;
                exec_stack.front().code = c2->car();
                exec_stack.front().evaled_args.clear();
            } else {
                exec_stack.front().mark = evaled;
                exec_stack.front().code = make_shared<nil>();
                exec_stack.front().evaled_args.clear();
            }
        } else {
            exec_stack.front().mark = evaluating;
            exec_stack.front().code = c->car();
            exec_stack.front().evaled_args.clear();
        }
    }
}

void eval_defun_special_form(std::deque<stackframe>& exec_stack) {
    shared_ptr<lispobj> lobj = exec_stack.front().code;
    shared_ptr<cons> c = dynamic_pointer_cast<cons>(lobj);
    if(!c) {
        throw string("define needs more arguments");
    }

    shared_ptr<symbol> funcname = dynamic_pointer_cast<symbol>(c->car());
    if(!funcname) {
        std::stringstream errormsg;
        errormsg << "defun: first argument must be symbol, instead got: ";
        c->car()->print(errormsg);
        errormsg << endl;
        throw errormsg.str();
    }

    shared_ptr<cons> c2 = dynamic_pointer_cast<cons>(c->cdr());
    if(!c2) {
        throw string("defun: not enough arguments");
    }

    if(typeid(*(c2->car())) != typeid(cons) &&
       typeid(*(c2->car())) != typeid(nil)) {
        throw string("defun: second argument not list");
    }

    if(typeid(*(c2->cdr())) != typeid(nil)) {
        cout << "defun: " << funcname->name() << " has no function body" << endl;
    }

    shared_ptr<lispfunc> lfunc(new lispfunc(c2->car(), exec_stack.front().scope, c2->cdr()));

    exec_stack.front().scope->defun(funcname->name(), lfunc);
    exec_stack.front().evaled_args.clear();
    exec_stack.front().mark = evaled;
    exec_stack.front().code = c->car();
}

void eval_defmacro_special_form(std::deque<stackframe>& exec_stack) {
    shared_ptr<lispobj> lobj = exec_stack.front().code;
    shared_ptr<cons> c = dynamic_pointer_cast<cons>(lobj);
    if(!c) {
        throw string("defmacro needs more arguments");
    }

    shared_ptr<symbol> macroname = dynamic_pointer_cast<symbol>(c->car());
    if(!macroname) {
        std::stringstream errormsg;
        errormsg << "defmacro: first argument must be symbol, instead got: ";
        c->car()->print(errormsg);
        errormsg << endl;
        throw errormsg.str();
    }

    shared_ptr<cons> c2 = dynamic_pointer_cast<cons>(c->cdr());
    if(!c2) {
        throw string("defmacro: not enough arguments");
    }

    if(typeid(*(c2->car())) != typeid(cons) &&
       typeid(*(c2->car())) != typeid(nil)) {
        throw string("defmacro: second argument not list");
    }

    if(typeid(*(c2->cdr())) == typeid(nil)) {
        cout << "defmacro: " << macroname->name() << " has no function body" << endl;
    }

    shared_ptr<macro> mac(new macro(c2->car(), exec_stack.front().scope, c2->cdr()));

    exec_stack.front().scope->defun(macroname->name(), mac);
    exec_stack.front().evaled_args.clear();
    exec_stack.front().mark = evaled;
    exec_stack.front().code = macroname;
}

void eval_lambda_special_form(std::deque<stackframe>& exec_stack) {
    shared_ptr<lispobj> lobj = exec_stack.front().code;
    shared_ptr<cons> c = dynamic_pointer_cast<cons>(lobj);
    if(!c) {
        throw string("lambda needs more arguments");
    }

    shared_ptr<cons> c2 = dynamic_pointer_cast<cons>(c->cdr());
    if(!c2) {
        throw string("lambda needs more arguments");
    }

    shared_ptr<lispfunc> lfunc(new lispfunc(c->car(), exec_stack.front().scope, c2));
    exec_stack.front().mark = evaled;
    exec_stack.front().code = lfunc;
}

void make_let_star_bindings(shared_ptr<lexicalscope> scope, shared_ptr<lispobj> bindings) {
    shared_ptr<cons> c = dynamic_pointer_cast<cons>(bindings);

    while(c) {
        shared_ptr<lispobj> binding = c->car();
        shared_ptr<symbol> symbol_binding = dynamic_pointer_cast<symbol>(binding);
        shared_ptr<cons> cons_binding = dynamic_pointer_cast<cons>(binding);

        if(symbol_binding) {
            scope->defval(symbol_binding->name(), make_shared<nil>());
        } else if(cons_binding) {
            shared_ptr<symbol> name = dynamic_pointer_cast<symbol>(cons_binding->car());
            if(!name) {
                throw string("invalid let binding variable");
            }

            shared_ptr<cons> c2 = dynamic_pointer_cast<cons>(cons_binding->cdr());
            if(c2) {
                shared_ptr<lispobj> val = eval(c2->car(), scope);
                if(val) {
                    scope->defval(name->name(), val);
                } else {
                    throw string("let binding eval failed");
                }
            } else {
                scope->defval(name->name(), make_shared<nil>());
            }
        } else {
            throw string("Invalid let binding");
        }

        bindings = c->cdr();
        c = dynamic_pointer_cast<cons>(bindings);
    }
}

void eval_let_star_special_form(std::deque<stackframe>& exec_stack) {
    //add new stack frame with new env, parent of current stack frame

    shared_ptr<lispobj> lobj = exec_stack.front().code;
    shared_ptr<cons> c = dynamic_pointer_cast<cons>(lobj);

    if(!c) {
        throw string("let needs more forms");
    }

    shared_ptr<lexicalscope> scope(new lexicalscope(exec_stack.front().scope));

    make_let_star_bindings(scope, c->car());

    exec_stack.front().code = c->cdr();
    exec_stack.front().scope = scope;
    exec_stack.front().mark = applying;
}

void eval_special_form(string name,
                      std::deque<stackframe>& exec_stack) {
    if(name == "if") {
        eval_if_special_form(exec_stack);
    } else if(name == "lambda") {
        eval_lambda_special_form(exec_stack);
    } else if(name == "module") {
        eval_module_special_form(exec_stack);
    } else if(name == "import") {
        shared_ptr<cons> c = dynamic_pointer_cast<cons>(exec_stack.front().code);
        if(!c) {
            throw string("import needs more arguments");
        }

        shared_ptr<module> m = nullptr; //env->get_module_by_prefix(c->car());
        if(!m) {
            stringstream errormsg;
            errormsg << "Module ";
            c->car()->print(errormsg);
            errormsg << " not found.";
            throw errormsg.str();
        } else if(!(m->init())) {
            stringstream errormsg;
            errormsg << "Module ";
            c->car()->print(errormsg);
            errormsg << " init failed.";
            throw errormsg.str();
        }
        exec_stack.front().scope->add_import(m);
        exec_stack.front().mark = evaled;
        exec_stack.front().code = make_shared<nil>();
    } else if(name == "begin") {
        exec_stack.front().mark = applying;
    } else if(name == "defun") {
        eval_defun_special_form(exec_stack);
    } else if(name == "defmacro") {
        eval_defmacro_special_form(exec_stack);
    } else if(name == "quote") {
        shared_ptr<cons> c = dynamic_pointer_cast<cons>(exec_stack.front().code);
        if(!c) {
            throw string("quote needs more arguments");
        }

        if(!dynamic_pointer_cast<nil>(c->cdr())) {
            throw string("quote has too many args");
        }

        exec_stack.front().code = c->car();
        exec_stack.front().mark = evaled;
    } else if(name == "let*") {
        eval_let_star_special_form(exec_stack);
    }
}

void evalstep(std::deque<stackframe>& exec_stack) {
    if(exec_stack.front().mark == evaled) {
        shared_ptr<lispobj> c = exec_stack.front().code;
        exec_stack.pop_front();
        if(exec_stack.front().mark == applying) {
            if(dynamic_pointer_cast<nil>(exec_stack.front().code)) {
                exec_stack.front().mark = evaled;
                exec_stack.front().code = c;
            } else {
                //the frame was popped, and nothing else needs doing
            }
        } else if(exec_stack.front().mark == evaluating ||
                  exec_stack.front().mark == evalspecial) {
            exec_stack.front().evaled_args.push_back(c);
        } else if(exec_stack.front().mark == evalmacro) {
            //This is the return of the macro expansion, so evaluate it
            exec_stack.front().mark = evaluating;
            exec_stack.front().code = c;
            exec_stack.front().evaled_args.clear();
        } else {
            throw string("bad stack");
        }
    } else if(exec_stack.front().mark == applying) {
        if(dynamic_pointer_cast<nil>(exec_stack.front().code)) {
            exec_stack.front().mark = evaled;
        } else if(shared_ptr<cons> c = dynamic_pointer_cast<cons>(exec_stack.front().code)) {
            shared_ptr<lispobj> next_statement = c->car();
            exec_stack.front().code = c->cdr();
            exec_stack.push_front(stackframe(exec_stack.front().scope,
                                             evaluating,
                                             next_statement));
        } else {
            throw string("bad stack");
        }
    } else if(exec_stack.front().mark == evaluating) {
        shared_ptr<cons> c = dynamic_pointer_cast<cons>(exec_stack.front().code);
        if(c) {
            //evaluating arguments
            shared_ptr<macro> macrofunc;
            if(exec_stack.front().evaled_args.size() == 0 &&
               is_special_form(c->car())) {
                //special forms go here, rather than normal argument evaluation
                exec_stack.front().mark = evalspecial;
                exec_stack.front().evaled_args.push_back(c->car());
                exec_stack.front().code = c->cdr();
            } else if(exec_stack.front().evaled_args.size() == 1 &&
                      (macrofunc = dynamic_pointer_cast<macro>(exec_stack.front().evaled_args.front()))) {
                exec_stack.front().mark = evalmacro;
                apply_macro(exec_stack);
            } else {
                exec_stack.front().code = c->cdr();
                exec_stack.push_front(stackframe(exec_stack.front().scope,
                                                 evaluating,
                                                 c->car()));
            }
        } else if(dynamic_pointer_cast<nil>(exec_stack.front().code)) {
            auto evaled_args = exec_stack.front().evaled_args;
            if(evaled_args.empty()) {
                throw string("empty function application");
            } else if(typeid(*(evaled_args.front())) == typeid(lispfunc)) {
                apply_lispfunc(exec_stack);
            } else if(shared_ptr<cfunc> func = dynamic_pointer_cast<cfunc>(evaled_args.front())) {
                evaled_args.erase(evaled_args.begin());
                shared_ptr<lispobj> ret = func->func(evaled_args);
                if(!ret) {
                    throw string("error in cfunc");
                }
                exec_stack.front().mark = evaled;
                exec_stack.front().code = ret;
                exec_stack.front().evaled_args.clear();
            } else {
                throw string("trying to apply a non-function");
            }
        } else if(shared_ptr<symbol> s = dynamic_pointer_cast<symbol>(exec_stack.front().code)) {
            //variable lookup
            exec_stack.front().mark = evaled;

            if(s->name() == "nil") {
                exec_stack.front().code = make_shared<nil>();
            } else if(s->name() == "t" || s->name()[0] == ':') {
                //self evaling
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
        shared_ptr<symbol> sym = dynamic_pointer_cast<symbol>(list_head);
        if(!sym) {
            throw string("non-special form given evalspecial mark.");
        }

        eval_special_form(sym->name(), exec_stack);
    } else {
        throw string("bad stack");
    }
}

shared_ptr<lispobj> eval(shared_ptr<lispobj> code,
                         shared_ptr<lexicalscope> tls) {
    std::deque<stackframe> exec_stack;

    exec_stack.push_front(stackframe(tls, evaluating, code));

    try {
        while(exec_stack.size() != 0 && (exec_stack.size() > 1 || exec_stack.front().mark != evaled)) {
            evalstep(exec_stack);
        }
    } catch(string error) {
        print_stack(exec_stack);
        cout << error << endl;
        return NULL;
    }

    return exec_stack.front().code;
}

shared_ptr<lispobj> plus(vector<shared_ptr<lispobj> > args) {
    int sum = 0;
    for(auto obj : args) {
        shared_ptr<number> n = dynamic_pointer_cast<number>(obj);
        if(!n) {
            throw string("plus requires numbers");
        }
        sum += n->value();
    }
    return make_shared<number>(sum);
}

shared_ptr<lispobj> minus(vector<shared_ptr<lispobj> > args) {
    if(args.size() > 1) {
        shared_ptr<number> n = dynamic_pointer_cast<number>(args[0]);
        if(!n) {
            throw string("minus requires numbers");
        }
        int diff = n->value();
        args.erase(args.begin());

        for(auto obj : args) {
            shared_ptr<number> n = dynamic_pointer_cast<number>(obj);
            if(!n) {
                throw string("minus requires numbers");
            }
            diff -= n->value();
        }
        return make_shared<number>(diff);
    } else {
        shared_ptr<lispobj> obj(args[0]);
        shared_ptr<number> n = dynamic_pointer_cast<number>(obj);
        if(!n) {
            throw string("minus requires numbers");
        }
        return make_shared<number>(-(n->value()));
    }
}

shared_ptr<lispobj> multiply(vector<shared_ptr<lispobj> > args) {
    int product = 1;
    for(auto obj : args) {
        shared_ptr<number> n = dynamic_pointer_cast<number>(obj);
        if(!n) {
            throw string("multiply requires numbers");
        }
        product *= n->value();
    }
    return make_shared<number>(product);
}

shared_ptr<lispobj> divide(vector<shared_ptr<lispobj> > args) {
    if(args.size() > 1) {
        shared_ptr<number> n = dynamic_pointer_cast<number>(args[0]);
        if(!n) {
            throw string("divide requires numbers");
        }
        int quotient = n->value();
        args.erase(args.begin());

        for(auto obj : args) {
            shared_ptr<number> n = dynamic_pointer_cast<number>(obj);
            if(!n) {
                throw string("divide requires numbers");
            }
            quotient /= n->value();
        }
        return make_shared<number>(quotient);
    } else {
        shared_ptr<lispobj> obj(args[0]);
        shared_ptr<number> n = dynamic_pointer_cast<number>(obj);
        if(!n) {
            throw string("divide requires numbers");
        }
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
        throw string("ERROR eq wants 2 args");
    }

    if(eq(args[0], args[1])) {
        return make_shared<symbol>("t");
    } else {
        return make_shared<nil>();
    }
}

shared_ptr<lispobj> eqv_cfunc(vector< shared_ptr<lispobj> > args) {
    if(args.size() != 2) {
        throw string("ERROR eqv wants 2 args");
    }

    if(eqv(args[0], args[1])) {
        return make_shared<symbol>("t");
    } else {
        return make_shared<nil>();
    }
}

shared_ptr<lispobj> equal_cfunc(vector< shared_ptr<lispobj> > args) {
    if(args.size() != 2) {
        throw string("ERROR equal wants 2 args");
    }

    if(equal(args[0], args[1])) {
        return make_shared<symbol>("t");
    } else {
        return make_shared<nil>();
    }
}

shared_ptr<lispobj> cons_cfunc(vector< shared_ptr<lispobj> > args) {
    if(args.size() != 2) {
        throw string("ERROR cons wants 2 args");
    }

    return make_shared<cons>(args[0], args[1]);
}

shared_ptr<lispobj> string_append_cfunc(vector< shared_ptr<lispobj> > args) {
    shared_ptr<lispstring> lstr(new lispstring(""));

    for(auto it = args.begin(); it != args.end(); ++it) {
        shared_ptr<lispstring> arg = dynamic_pointer_cast<lispstring>(*it);
        if(!arg) {
            throw string("ERROR string-append wants only strings");
        }

        lstr->append(arg);
    }

    return lstr;
}

shared_ptr<lispobj> open_file_cfunc(vector< shared_ptr<lispobj> > args) {
    if(args.size() != 1 || !dynamic_pointer_cast<lispstring>(args[0])) {
        throw string("ERROR open-file wants one string");
    }

    shared_ptr<lispstring> filename = dynamic_pointer_cast<lispstring>(args[0]);
    return make_shared<fileinputport>(filename->get_contents());
}

shared_ptr<lispobj> read_cfunc(vector< shared_ptr<lispobj> > args) {
    if(args.size() != 1 || !dynamic_pointer_cast<fileinputport>(args[0])) {
        throw string("ERROR read wants on input-port");
    }

    shared_ptr<fileinputport> port = dynamic_pointer_cast<fileinputport>(args[0]);
    return port->read();
}

shared_ptr<lispobj> car_cfunc(vector< shared_ptr<lispobj> > args) {
    if(args.size() != 1 || !dynamic_pointer_cast<cons>(args[0])) {
        throw string("ERROR car wants one cons");
    }

    shared_ptr<cons> c = dynamic_pointer_cast<cons>(args[0]);
    return c->car();
}

shared_ptr<lispobj> cdr_cfunc(vector< shared_ptr<lispobj> > args) {
    if(args.size() != 1 || !dynamic_pointer_cast<cons>(args[0])) {
        throw string("ERROR cdr wants one cons");
    }

    shared_ptr<cons> c = dynamic_pointer_cast<cons>(args[0]);
    return c->cdr();
}

shared_ptr<lispobj> consp_cfunc(vector< shared_ptr<lispobj> > args) {
    if(args.size() != 1) {
        throw string("ERROR cons? wants one argument");
    }

    if(dynamic_pointer_cast<cons>(args[0]) != nullptr) {
        return make_shared<symbol>("t");
    } else {
        return make_shared<nil>();
    }
}

shared_ptr<module> make_builtins_module(shared_ptr<lexicalscope> top_level_scope) {
    shared_ptr<lispobj> module_name(new cons(make_shared<symbol>("builtins"), make_shared<nil>()));
    shared_ptr<module> builtins_module(new module(module_name, top_level_scope));
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
    builtins_module->defun_and_export("string-append", make_shared<cfunc>(string_append_cfunc));
    builtins_module->defun_and_export("open-file", make_shared<cfunc>(open_file_cfunc));
    builtins_module->defun_and_export("read", make_shared<cfunc>(read_cfunc));
    builtins_module->defun_and_export("car", make_shared<cfunc>(car_cfunc));
    builtins_module->defun_and_export("cdr", make_shared<cfunc>(cdr_cfunc));
    builtins_module->defun_and_export("cons?", make_shared<cfunc>(consp_cfunc));

    return builtins_module;
}

bool istrue(shared_ptr<lispobj> lobj) {
    return !dynamic_pointer_cast<nil>(lobj);
}
