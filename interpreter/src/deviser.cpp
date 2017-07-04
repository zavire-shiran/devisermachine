#include "deviser.hpp"

#include <map>
#include <vector>
#include <iostream>
#include <utility>
#include <sstream>

#include "bytecode.hpp"

using std::vector;
using std::map;
using std::string;
using std::cout;
using std::endl;

dvs alloc_dvs(deviserstate* dstate);
dvs get_lfunc_name(dvs lfunc);
std::shared_ptr<module_info> make_module(deviserstate* dstate, dvs name);

void internal_print(dvs obj, std::ostream& out);

const dvs_int null_typeid = 1;
const dvs_int int_typeid = 2;
const dvs_int symbol_typeid = 3;
const dvs_int cfunc_typeid = 4;
const dvs_int lfunc_typeid = 5;
const dvs_int module_typeid = 6;

bool is_null(dvs d) {
    return d == nullptr;
}

bool is_cons(dvs d) {
    return !is_null(d) && (reinterpret_cast<dvs_int>(d->car) & 0x1) == 0;
}

bool is_list(dvs d) {
    return is_null(d) || is_cons(d);
}

bool is_marked(dvs d) {
    return (reinterpret_cast<dvs_int>(d->car) & 0x2) == 2;
}

void set_typeid(dvs d, dvs_int tid) {
    d->car = reinterpret_cast<dvs>(tid << 2 | 0x1);
}

dvs_int get_typeid(dvs d) {
    if(is_null(d)) {
        return null_typeid;
    } else if((reinterpret_cast<dvs_int>(d->car) & 0x1) != 0x1) {
        return 0;
    } else {
        return reinterpret_cast<dvs_int>(d->car) >> 2;
    }
}

bool is_int(dvs d) {
    return get_typeid(d) == int_typeid;
}

bool is_symbol(dvs d) {
    return get_typeid(d) == symbol_typeid;
}

bool is_cfunc(dvs d) {
    return get_typeid(d) == cfunc_typeid;
}

bool is_lfunc(dvs d) {
    return get_typeid(d) == lfunc_typeid;
}

bool is_module(dvs d) {
    return get_typeid(d) == module_typeid;
}

string symbol_string(dvs d) {
    return *reinterpret_cast<string*>(d->cdr);
}

bool issymchar(int c) {
    return !isspace(c) && isprint(c) && c != '(' && c != ')';
}

dvs_int get_int(dvs d) {
    if(is_int(d)) {
        return reinterpret_cast<dvs_int>(d->cdr);
    } else {
        throw "not an integer";
    }
}

struct lfunc_info {
    dvs name; // but not all functions have names...
    uint64_t num_args;
    uint64_t num_var;
    vector<bytecode> bytecode;
    vector<dvs> constants;
    std::shared_ptr<module_info> module;
};

dvs get_lfunc_name(dvs lfunc) {
    if(is_lfunc(lfunc)) {
        lfunc_info* finfo = reinterpret_cast<lfunc_info*>(lfunc->cdr);
        return finfo->name;
    } else {
        throw "not an lfunc";
    }
}

struct cfunc_info {
    cfunc_type func_ptr;
    std::shared_ptr<module_info> module;
};

deviserstate* create_deviser_state() {
    deviserstate* dstate = new deviserstate;
    dstate->memoryarenasize = 1000;
    dstate->nextfree = 0;
    dstate->memoryarena = new deviserobj[dstate->memoryarenasize];
    dstate->stack.push_back(stackframe());
    stackframe& tsf = dstate->stack.back();
    tsf.variables.insert(tsf.variables.begin(), 3, nullptr);
    return dstate;
}

dvs alloc_dvs(deviserstate* dstate) {
    if(dstate->nextfree < dstate->memoryarenasize) {
        stackframe& currentframe = dstate->stack.back();
        dvs newalloc = dstate->memoryarena + dstate->nextfree++;
        currentframe.workstack.push_back(newalloc);
        return newalloc;
    } else {
        return nullptr;
    }
}

void call_function(deviserstate* dstate, uint64_t argc) {
    stackframe& currentframe = dstate->stack[dstate->stack.size() - 1];
    uint64_t stacksize = currentframe.workstack.size();
    if(currentframe.workstack.size() < argc + 1) {
        throw "invalid function call: not enough args on stack";
    }

    auto begin_args_iter = currentframe.workstack.end();
    auto end_args_iter = currentframe.workstack.end();
    for(uint64_t i = 0; i < argc; ++i) {
        --begin_args_iter;
    }

    dvs function = currentframe.workstack[stacksize - (argc+1)];
    stackframe newframe;
    newframe.pc = 0;

    if(is_cfunc(function)) {
        newframe.workstack.insert(newframe.workstack.begin(), begin_args_iter, end_args_iter);
        newframe.variables.insert(newframe.variables.begin(), 3, nullptr);
        currentframe.workstack.erase(begin_args_iter, end_args_iter);
        pop(dstate);

        cfunc_info* finfo = reinterpret_cast<cfunc_info*>(function->cdr);
        newframe.module = finfo->module;

        dstate->stack.push_back(newframe);
        (*finfo->func_ptr)(dstate);
        return_function(dstate);
    } else if(is_lfunc(function)) {
        lfunc_info* finfo = reinterpret_cast<lfunc_info*>(function->cdr);
        newframe.variables.insert(newframe.variables.begin(), begin_args_iter, end_args_iter);
        while(newframe.variables.size() < finfo->num_var) {
            newframe.variables.push_back(nullptr);
        }
        newframe.bytecode = finfo->bytecode;
        newframe.constants = finfo->constants;
        newframe.module = finfo->module;
        currentframe.workstack.erase(begin_args_iter, end_args_iter);
        pop(dstate);
        dstate->stack.push_back(newframe);
    } else {
        throw "not a function";
    }
}

void return_function(deviserstate* dstate) {
    if(dstate->stack.size() < 2) {
        throw "can't return from top level";
    }

    dvs retval = *(dstate->stack.back().workstack.rbegin());
    dstate->stack.pop_back();
    dstate->stack.back().workstack.push_back(retval);
}

void destroy_deviser_state(deviserstate* ds) {
    delete ds;
}

uint64_t workstacksize(deviserstate* dstate) {
    return dstate->stack.back().workstack.size();
}

void rot_two(deviserstate* dstate) {
    stackframe& currentframe = dstate->stack.back();
    uint64_t stacksize = currentframe.workstack.size();
    dvs temp = currentframe.workstack[stacksize-1];
    currentframe.workstack[stacksize-1] = currentframe.workstack[stacksize-2];
    currentframe.workstack[stacksize-2] = temp;
}

dvs pop(deviserstate* dstate) {
    dvs back = dstate->stack.back().workstack.back();
    dstate->stack.back().workstack.pop_back();
    return back;
}

void push(deviserstate* dstate, dvs val) {
    stackframe& currentframe = dstate->stack.back();
    currentframe.workstack.push_back(val);
}

void dup(deviserstate* dstate) {
    stackframe& currentframe = dstate->stack.back();
    uint64_t stacksize = currentframe.workstack.size();
    if(stacksize > 0) {
        currentframe.workstack.push_back(currentframe.workstack[stacksize-1]);
    } else {
        throw "nothing to dup";
    }
}

bool read(deviserstate* dstate, const std::string& in) {
    std::stringstream ss(in);
    return read(dstate, ss);
}

bool read(deviserstate* dstate, std::istream& in) {
    char c = static_cast<char>(in.peek());
    while(isspace(c)) {
        in.get();
        c = static_cast<char>(in.peek());
    }

    if(c == '(') {
        int num = 0;
        in.get();
        c = static_cast<char>(in.peek());
        while(isspace(c)) {
            in.get();
            c = static_cast<char>(in.peek());
        }
        while(c != ')') {
            read(dstate, in);
            num++;

            c = static_cast<char>(in.peek());
            while(isspace(c)) {
                in.get();
                c = static_cast<char>(in.peek());
            }
        }
        in.get();

        push_null(dstate);

        for(int i = 0; i < num; ++i) {
            make_cons(dstate);
        }
    } else if(isdigit(c)) {
        string num(1, c);
        in.get();
        while(isdigit(in.peek())) {
            num.append(1, static_cast<char>(in.get()));
        }

        push_int(dstate, stol(num));
    } else if(issymchar(c)) {
        string sym(1, c);
        in.get();
        while(issymchar(in.peek())) {
            sym.append(1, static_cast<char>(in.get()));
        }

        push_symbol(dstate, sym);
    } else {
        return false;
    }
    return true;
}

void internal_print(dvs obj, std::ostream& out) {
    if(is_null(obj)) {
        out << "()";
    }
    else if(is_cons(obj)) {
        out << "(";
        while(obj != nullptr and is_cons(obj)) {
            internal_print(obj->pcar(), out);
            obj = obj->cdr;
            if(obj != nullptr) {
                out << " ";
            }
        }

        if(obj != nullptr) {
            out << ". ";
            internal_print(obj, out);
        }
        out << ")";
    } else {
        switch(get_typeid(obj)) {
        case int_typeid:
            out << get_int(obj);
            break;
        case symbol_typeid:
        {
            string name = *reinterpret_cast<string*>(obj->cdr);
            out << name;
            break;
        }
        case cfunc_typeid:
            out << "<cfunc " << obj->cdr << ">";
            break;
        case lfunc_typeid:
            out << "<lfunc " << obj->cdr << ">";
            break;
        default:
            throw "unknown typeid to print";
        }
    }
}

void print(deviserstate* dstate, std::ostream& out) {
    stackframe& currentframe = dstate->stack.back();
    uint64_t stacksize = currentframe.workstack.size();
    if(stacksize == 0) {
        throw "nothing to print";
    }

    dvs obj = currentframe.workstack[stacksize - 1];
    internal_print(obj, out);
}

void push_int(deviserstate* dstate, dvs_int value) {
    dvs newint = alloc_dvs(dstate);
    set_typeid(newint, int_typeid);
    newint->cdr = reinterpret_cast<dvs>(value);
}

dvs_int get_int_value(deviserstate* dstate, uint64_t pos) {
    stackframe& currentframe = dstate->stack.back();
    uint64_t int_pos = currentframe.workstack.size() - (pos + 1);
    if(int_pos >= currentframe.workstack.size()) {
        throw "stack pos out of range";
    }
    dvs i = currentframe.workstack[int_pos];
    return get_int(i);
}

void push_null(deviserstate* dstate) {
    dstate->stack.back().workstack.push_back(nullptr);
}

void make_cons(deviserstate* dstate) {
    stackframe& currentframe = dstate->stack.back();
    if(currentframe.workstack.size() < 2) {
        throw "stack too small for cons";
    }

    uint64_t stacksize = currentframe.workstack.size();
    dvs car = currentframe.workstack[stacksize - 2];
    dvs cdr = currentframe.workstack[stacksize - 1];

    dvs newcons = alloc_dvs(dstate);
    newcons->car = car;
    newcons->cdr = cdr;

    rot_two(dstate);
    pop(dstate);
    rot_two(dstate);
    pop(dstate);
}

void cons_car(deviserstate* dstate, uint64_t pos) {
    stackframe& currentframe = dstate->stack.back();
    if(currentframe.workstack.size() < pos) {
        throw "invalid position for cons_car";
    }

    uint64_t cons_pos = currentframe.workstack.size() - (pos + 1);
    dvs conscell = currentframe.workstack[cons_pos];
    if(!is_cons(conscell)) {
        throw "not a cons";
    }

    currentframe.workstack.push_back(conscell->pcar());
}

void cons_cdr(deviserstate* dstate, uint64_t pos) {
    stackframe& currentframe = dstate->stack.back();
    if(currentframe.workstack.size() < pos) {
        throw "invalid position for cons_cdr";
    }

    uint64_t cons_pos = currentframe.workstack.size() - (pos + 1);
    dvs conscell = currentframe.workstack[cons_pos];
    if(!is_cons(conscell)) {
        throw "not a cons";
    }

    currentframe.workstack.push_back(conscell->cdr);
}

void push_symbol(deviserstate* dstate, string symbolname) {
    auto symbol_entry = dstate->symbol_table.find(symbolname);
    dvs symbol = nullptr;
    if(symbol_entry == dstate->symbol_table.end()) {
        symbol = alloc_dvs(dstate);
        set_typeid(symbol, symbol_typeid);
        symbol->cdr = reinterpret_cast<dvs>(new string(symbolname));
        dstate->symbol_table.insert(std::make_pair(symbolname, symbol));
    } else {
        symbol = symbol_entry->second;
        dstate->stack.back().workstack.push_back(symbol);
    }
}

string get_symbol_name(deviserstate* dstate, uint64_t pos) {
    stackframe& currentframe = dstate->stack.back();
    if(currentframe.workstack.size() < pos) {
        throw "invalid position for get_symbol_name";
    }

    uint64_t symbol_pos = currentframe.workstack.size() - (pos + 1);
    dvs symbol = currentframe.workstack[symbol_pos];
    if(is_symbol(symbol)) {
        return *reinterpret_cast<string*>(symbol->cdr);
    } else {
        throw "not a symbol";
    }
}

void push_cfunc(deviserstate* dstate, cfunc_type func, std::shared_ptr<module_info> mod) {
    dvs cfunc = alloc_dvs(dstate);
    set_typeid(cfunc, cfunc_typeid);
    cfunc_info* finfo = new cfunc_info;
    finfo->func_ptr = func;
    finfo->module = mod;
    cfunc->cdr = reinterpret_cast<dvs>(finfo);
}

void generate_lfunc(deviserstate* dstate, dvs name, uint64_t num_args, uint64_t num_var,
                    const vector<dvs>& constants,
                    const vector<bytecode>& bytecode,
                    const std::shared_ptr<module_info> mod) {
    dvs lfunc = alloc_dvs(dstate);
    set_typeid(lfunc, lfunc_typeid);
    lfunc_info* finfo = new lfunc_info;
    finfo->name = name;
    finfo->num_args = num_args;
    finfo->num_var = num_var;
    finfo->bytecode = bytecode;
    finfo->constants = constants;
    finfo->module = mod;
    lfunc->cdr = reinterpret_cast<dvs>(finfo);
}

void print_lfunc_info(deviserstate* dstate) {
    stackframe& currentframe = dstate->stack.back();
    dvs lfunc = currentframe.workstack.back();
    if(!(is_lfunc(lfunc))) {
        return;
    }

    lfunc_info* lfi = reinterpret_cast<lfunc_info*>(lfunc->cdr);
    cout << "numargs: " << lfi->num_args << " numvars: " << lfi->num_var << endl;
    cout << "constants:" << endl;
    for(dvs constant : lfi->constants) {
        internal_print(constant, cout);
        cout << endl;
    }
    cout << "bytecode:" << endl;
    disassemble_bytecode(lfi->bytecode, cout);
}

void store_variable(deviserstate* dstate, uint64_t varnum) {
    stackframe& currentframe = dstate->stack.back();
    if(varnum >= currentframe.variables.size()) {
        throw "out of range variable store";
    }
    if(currentframe.workstack.empty()) {
        throw "no value to store in variable";
    }

    currentframe.variables[varnum] = currentframe.workstack.back();
    pop(dstate);
}

void load_variable(deviserstate* dstate, uint64_t varnum) {
    stackframe& currentframe = dstate->stack.back();
    if(varnum >= currentframe.variables.size()) {
        throw "out of range variable load";
    }

    currentframe.workstack.push_back(currentframe.variables[varnum]);
}

void store_global(deviserstate* dstate, map<dvs,dvs>& top_level_env) {
    stackframe& currentframe = dstate->stack.back();
    size_t stacksize = currentframe.workstack.size();
    auto insert_status = top_level_env.insert(std::make_pair(
                                                  currentframe.workstack[stacksize-2],
                                                  currentframe.workstack[stacksize-1]));
    if(!insert_status.second) {
        insert_status.first->second = currentframe.workstack[stacksize-1];
    }

    pop(dstate);
    pop(dstate);
}

void load_global(deviserstate* dstate, map<dvs,dvs>& top_level_env) {
    stackframe& currentframe = dstate->stack.back();
    size_t stacksize = currentframe.workstack.size();
    dvs varname = currentframe.workstack[stacksize-1];
    auto binding = top_level_env.find(varname);
    pop(dstate);
    if(binding != top_level_env.end()) {
        currentframe.workstack.push_back(binding->second);
    } else {
        throw "cannot find top level var";
    }
}

void store_module_var(deviserstate* dstate) {
    stackframe& currentframe = dstate->stack.back();
    store_global(dstate, currentframe.module->value_bindings);
}

void load_module_var(deviserstate* dstate) {
    stackframe& currentframe = dstate->stack.back();
    load_global(dstate, currentframe.module->value_bindings);
}

void store_module_func(deviserstate* dstate) {
    stackframe& currentframe = dstate->stack.back();
    store_global(dstate, currentframe.module->func_bindings);
}

void load_module_func(deviserstate* dstate) {
    stackframe& currentframe = dstate->stack.back();
    load_global(dstate, currentframe.module->func_bindings);
}

void push_constant(deviserstate* dstate, uint64_t constnum) {
    stackframe& currentframe = dstate->stack.back();
    if(constnum < currentframe.constants.size()) {
        currentframe.workstack.push_back(currentframe.constants[constnum]);
    } else {
        throw "constnum out of range";
    }
}

void dump_stack(deviserstate* dstate) {
    cout << "stack:" << endl;
    for(stackframe& frame : dstate->stack) {
        cout << "frame:" << endl;
        cout << "pc: " << frame.pc << endl;
        cout << "workstack:" << endl;
        for(dvs item : frame.workstack) {
            internal_print(item, cout);
            cout << endl;
        }
    }
    cout << endl;
}

std::shared_ptr<module_info> make_module(deviserstate* dstate, dvs name) {
    std::shared_ptr<module_info> module = std::make_shared<module_info>();
    module->name = name;
    dstate->modules.insert(make_pair(name, module));
    return module;
}

std::shared_ptr<module_info> get_module(deviserstate* dstate, std::string name) {
    push_symbol(dstate, name);
    dvs sym_name = pop(dstate);
    auto it = dstate->modules.find(sym_name);
    if(it == dstate->modules.end()) {
        return make_module(dstate, sym_name);
    } else {
        return it->second;
    }
}

void load_module(deviserstate* dstate, std::string modulestring) {
    read(dstate, modulestring);
    stackframe& currentframe = dstate->stack.back();
    dvs modulesrc = currentframe.workstack.back();

    if(!is_cons(modulesrc)) {
        throw "module source should be a list";
    }

    //TODO: check that module car is 'module

    modulesrc = modulesrc->cdr;
    if(modulesrc == nullptr) {
        throw "module source too short";
    }

    dvs modulename = modulesrc->pcar();
    modulesrc = modulesrc->cdr;

    if(!is_symbol(modulename)) {
        throw "module name is not a symbol";
    }

    //inefficiency: we are pulling the string out of a symbol here, but it
    //gets turned back into a symbol in get_module().
    std::shared_ptr<module_info> module = get_module(dstate, symbol_string(modulename));

    while(modulesrc != nullptr) {
        if(!is_cons(modulesrc)) {
            throw "module should be a list";
        }

        dvs item = modulesrc->pcar();
        if(!is_cons(item)) {
            throw "module items should be a list";
        }

        dvs itemcar = item->pcar();
        if(!is_symbol(itemcar)) {
            throw "module items should start with a symbol";
        }

        if(symbol_string(itemcar) == "defun") {
            push(dstate, item->cdr);
            compile_function(dstate, module);
            dvs lfunc = pop(dstate);
            dvs name = get_lfunc_name(lfunc);
            module->func_bindings.insert(std::make_pair(name, lfunc));
        }
        modulesrc = modulesrc->cdr;
    }
}

void set_module(deviserstate* dstate, std::string module_name) {
    stackframe& currentframe = dstate->stack.back();
    currentframe.module = get_module(dstate, module_name);
}

void eval(deviserstate* dstate) {
    stackframe& currentframe = dstate->stack.back();
    dvs expression = currentframe.workstack.back();
    if(is_cons(expression) &&
       is_symbol(expression->pcar())) {
        if(symbol_string(expression->pcar()) == "defun") {
            throw "don't know how to do defun yet";
            return;
        } else if(symbol_string(expression->pcar()) == "in-module") {
            expression = expression->cdr;
            if(is_cons(expression) && is_symbol(expression->pcar())) {
                set_module(dstate, symbol_string(expression->pcar()));
                pop(dstate);
                push(dstate, expression->pcar());
            } else {
                throw "malformed in-module";
            }
            return;
        }
    }
    push_null(dstate);
    make_cons(dstate);

    push_null(dstate);
    rot_two(dstate);
    make_cons(dstate);

    push_symbol(dstate, "lambda");
    rot_two(dstate);
    make_cons(dstate);
    //print(dstate, std::cout);

    compile_function(dstate, currentframe.module);
    //print_lfunc_info(dstate);
    call_function(dstate, 0);
    run_bytecode(dstate);
}
