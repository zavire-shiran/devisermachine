#include "deviser.hpp"

#include <map>
#include <vector>
#include <iostream>
#include <utility>
#include <sstream>

using std::vector;
using std::map;
using std::string;
using std::cout;
using std::endl;

struct deviserobj;
typedef deviserobj* dvs;

dvs alloc_dvs(deviserstate* dstate);

void internal_print(dvs obj, std::ostream& out);

bool is_null(dvs d);
bool is_cons(dvs d);
bool is_marked(dvs d);

void set_typeid(dvs d, dvs_int tid);
dvs_int get_typeid(dvs d);

bool is_symbol(dvs d);
bool is_int(dvs d);
dvs_int get_int(dvs d);

const dvs_int int_typeid = 1;
const dvs_int symbol_typeid = 2;
const dvs_int cfunc_typeid = 3;

struct deviserobj {
    dvs pcar() {
        // clear the low 2 bits
        return reinterpret_cast<dvs>(reinterpret_cast<dvs_int>(car) & (~0x3));
    }
    dvs car;
    dvs cdr;
};

bool is_null(dvs d) {
    return d == nullptr;
}

bool is_cons(dvs d) {
    return (reinterpret_cast<dvs_int>(d->car) & 0x1) == 0;
}

bool is_marked(dvs d) {
    return (reinterpret_cast<dvs_int>(d->car) & 0x2) == 2;
}

void set_typeid(dvs d, dvs_int tid) {
    d->car = reinterpret_cast<dvs>(tid << 2 | 0x1);
}

dvs_int get_typeid(dvs d) {
    return reinterpret_cast<dvs_int>(d->car) >> 2;
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
dvs_int get_int(dvs d) {
    if(is_int(d)) {
        return reinterpret_cast<dvs_int>(d->cdr);
    } else {
        throw "not an integer";
    }
}

struct stackframe {
    vector<dvs> workstack;
};

struct deviserstate {
    dvs memoryarena;
    size_t memoryarenasize;
    size_t nextfree;
    vector<stackframe> stack;
    map<string, dvs> symbol_table;
};

deviserstate* create_deviser_state() {
    deviserstate* dstate = new deviserstate;
    dstate->memoryarenasize = 1000;
    dstate->nextfree = 0;
    dstate->memoryarena = new deviserobj[dstate->memoryarenasize];
    dstate->stack.push_back(stackframe());
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
    newframe.workstack.insert(newframe.workstack.begin(), begin_args_iter, end_args_iter);
    dump_stack(dstate);
    currentframe.workstack.erase(begin_args_iter, end_args_iter);
    pop(dstate);
    dstate->stack.push_back(newframe);

    if(is_cfunc(function)) {
        cfunc_type cfunc = reinterpret_cast<cfunc_type>(function->cdr);
        (*cfunc)(dstate);
        return_function(dstate);
    } else {
        cout << "called function ";
        internal_print(function, cout);
        cout << " with stack" << endl;
        dump_stack(dstate);
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

void pop(deviserstate* dstate) {
    dstate->stack.back().workstack.pop_back();
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

void read(deviserstate* dstate, const std::string& in) {
    std::stringstream ss(in);
    read(dstate, ss);
}

void read(deviserstate* dstate, std::istream& in) {
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
    } else if(isalpha(c)) {
        string sym(1, c);
        in.get();
        while(isalnum(in.peek())) {
            sym.append(1, static_cast<char>(in.get()));
        }

        push_symbol(dstate, sym);
    }
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

void push_cfunc(deviserstate* dstate, cfunc_type func) {
    dvs cfunc = alloc_dvs(dstate);
    set_typeid(cfunc, cfunc_typeid);
    cfunc->cdr = reinterpret_cast<dvs>(func);
}

void dump_stack(deviserstate* dstate) {
    cout << "stack:" << endl;
    for(stackframe& frame : dstate->stack) {
        cout << "frame:" << endl;
        for(dvs item : frame.workstack) {
            internal_print(item, cout);
            cout << endl;
        }
    }
    cout << endl;
}
