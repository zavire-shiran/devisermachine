#pragma once
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>
#include <map>
#include <forward_list>

#include "types.hpp"

struct deviserobj {
    dvs pcar() {
        // clear the low 2 bits
        return reinterpret_cast<dvs>(reinterpret_cast<dvs_int>(car) & (~0x3));
    }
    dvs car;
    dvs cdr;
};

struct stackframe {
    std::vector<dvs> workstack;
    std::vector<dvs> variables;
    std::vector<dvs> constants;
    std::vector<bytecode> bytecode;
    uint64_t pc;
    std::shared_ptr<module_info> module; // eventually should include closure information
};

struct memoryarena {
    dvs arena;
    size_t size;

    memoryarena(size_t _size) {
        size = _size;
        arena = new deviserobj[size];
    }
};

struct deviserstate {
    std::vector<memoryarena> memoryarenas;
    std::forward_list<dvs> freelist;
    std::vector<stackframe> stack;
    std::map<std::string, dvs> symbol_table;
    std::map<dvs, std::shared_ptr<module_info> > modules;
};

struct module_info {
    dvs name;
    std::map<dvs, dvs> value_bindings;
    std::map<dvs, dvs> func_bindings;
    std::vector<std::shared_ptr<module_info> > imports;
};

const dvs_int cons_typeid = 0;
const dvs_int null_typeid = 1;
const dvs_int int_typeid = 2;
const dvs_int symbol_typeid = 3;
const dvs_int cfunc_typeid = 4;
const dvs_int lfunc_typeid = 5;
const dvs_int module_typeid = 6;
const dvs_int macro_typeid = 7;
const dvs_int free_typeid = 8; // this can't be the best way to do this...

struct lfunc_info {
    dvs name; // but not all functions have names...
    uint64_t num_args;
    uint64_t num_var;
    bool has_rest;
    std::vector<bytecode> bytecode;
    std::vector<dvs> constants;
    std::shared_ptr<module_info> module;
};

struct cfunc_info {
    cfunc_type func_ptr;
    std::shared_ptr<module_info> module;
};

bool is_null(dvs d);
bool is_cons(dvs d);
bool is_list(dvs d);
bool is_marked(dvs d);
bool is_free(dvs d);

void mark(dvs d);
void unmark(dvs d);
void free(dvs d);

void set_typeid(dvs d, dvs_int tid);
dvs_int get_typeid(dvs d);

bool is_symbol(dvs d);
bool is_int(dvs d);
bool is_cfunc(dvs d);
bool is_lfunc(dvs d);
bool is_module(dvs d);
bool is_macro(dvs d);
bool is_func(dvs d);

std::string symbol_string(dvs d);
bool issymchar(int c);

dvs_int get_int(dvs d);

deviserstate* create_deviser_state();
void destroy_deviser_state(deviserstate*);

void call_function(deviserstate* dstate, uint64_t argc);
void return_function(deviserstate* dstate);

uint64_t workstacksize(deviserstate* dstate);
void rot_two(deviserstate* dstate);
dvs pop(deviserstate* dstate);
void push(deviserstate* dstate, dvs val);
void dup(deviserstate* dstate);

bool read(deviserstate* dstate, const std::string& in);
bool read(deviserstate* dstate, std::istream& in);
void print(deviserstate* dstate, std::ostream& out);

void push_int(deviserstate* dstate, dvs_int value);
dvs_int get_int_value(deviserstate* dstate, uint64_t pos);

void push_null(deviserstate* dstate);
void make_cons(deviserstate* dstate);
void cons_car(deviserstate* dstate, int position = 0);
void cons_cdr(deviserstate* dstate, int position = 0);

void push_symbol(deviserstate* dstate, std::string symbolname);
std::string get_symbol_name(deviserstate* dstate, uint64_t pos);

void push_cfunc(deviserstate* dstate, cfunc_type func, std::shared_ptr<module_info> mod);

void generate_lfunc(deviserstate* dstate, dvs name, uint64_t num_args, uint64_t num_var,
                    const std::vector<dvs>& constants,
                    const std::vector<bytecode>& bytecode,
                    bool has_rest,
                    const std::shared_ptr<module_info> mod);
void generate_macro(deviserstate* dstate, dvs name, uint64_t num_args, uint64_t num_var,
                    const std::vector<dvs>& constants,
                    const std::vector<bytecode>& bytecode,
                    bool has_rest,
                    const std::shared_ptr<module_info> mod);
void print_lfunc_info(deviserstate* dstate);

void store_variable(deviserstate* dstate, uint64_t varnum);
void load_variable(deviserstate* dstate, uint64_t varnum);

void store_global(deviserstate* dstate, std::map<dvs,dvs>& top_level_env);
bool load_global(deviserstate* dstate, std::map<dvs,dvs>& top_level_env);

void store_module_var(deviserstate* dstate);
void load_module_var(deviserstate* dstate);
void store_module_func(deviserstate* dstate);
void load_module_func(deviserstate* dstate);

void push_constant(deviserstate* dstate, uint64_t constnum);

void dump_stack(deviserstate* dstate);

void defun(deviserstate* dstate, std::shared_ptr<module_info> module, dvs expr);
void defmacro(deviserstate* dstate, std::shared_ptr<module_info> module,  dvs expr);

std::shared_ptr<module_info> get_module(deviserstate* dstate, std::string name);
void load_module(deviserstate* dstate, std::string modulesrc);
void set_module(deviserstate* dstate, std::string module_name);

void eval(deviserstate* dstate);
void macroexpand1(deviserstate* dstate);
void macroexpand(deviserstate* dstate);

size_t stack_index(deviserstate* dstate, int position);
size_t stack_size(deviserstate* dstate);

bool listp(deviserstate* dstate, int position = 0);
bool consp(deviserstate* dstate, int position = 0);
bool nullp(deviserstate* dstate, int position = 0);

void lisp_car(deviserstate* dstate);
void lisp_cdr(deviserstate* dstate);
void lisp_cons(deviserstate* dstate);
void lisp_consp(deviserstate* dstate);
void lisp_list(deviserstate* dstate);
