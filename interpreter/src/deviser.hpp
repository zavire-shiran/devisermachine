#pragma once
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>
#include <map>

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
};

struct deviserstate {
    dvs memoryarena;
    size_t memoryarenasize;
    size_t nextfree;
    std::vector<stackframe> stack;
    std::map<std::string, dvs> symbol_table;
    std::map<dvs, dvs> top_level_var_env;
    std::map<dvs, dvs> top_level_func_env;
};

bool is_null(dvs d);
bool is_cons(dvs d);
bool is_list(dvs d);
bool is_marked(dvs d);

void set_typeid(dvs d, dvs_int tid);
dvs_int get_typeid(dvs d);

bool is_symbol(dvs d);
bool is_int(dvs d);
bool is_cfunc(dvs d);
bool is_lfunc(dvs d);

std::string symbol_string(dvs d);

dvs_int get_int(dvs d);

deviserstate* create_deviser_state();
void destroy_deviser_state(deviserstate*);

void call_function(deviserstate* dstate, uint64_t argc);
void return_function(deviserstate* dstate);

uint64_t workstacksize(deviserstate* dstate);
void rot_two(deviserstate* dstate);
void pop(deviserstate* dstate);
void dup(deviserstate* dstate);

void read(deviserstate* dstate, const std::string& in);
void read(deviserstate* dstate, std::istream& in);
void print(deviserstate* dstate, std::ostream& out);

void push_int(deviserstate* dstate, dvs_int value);
dvs_int get_int_value(deviserstate* dstate, uint64_t pos);

void push_null(deviserstate* dstate);
void make_cons(deviserstate* dstate);
void cons_car(deviserstate* dstate, uint64_t pos);
void cons_cdr(deviserstate* dstate, uint64_t pos);

void push_symbol(deviserstate* dstate, std::string symbolname);
std::string get_symbol_name(deviserstate* dstate, uint64_t pos);

void push_cfunc(deviserstate* dstate, cfunc_type func);

void generate_lfunc(deviserstate* dstate, uint64_t num_args, uint64_t num_var,
                    const std::vector<dvs>& constants,
                    const std::vector<bytecode>& bytecode);
void print_lfunc_info(deviserstate* dstate);

void store_variable(deviserstate* dstate, uint64_t varnum);
void load_variable(deviserstate* dstate, uint64_t varnum);

void store_global(deviserstate* dstate, std::map<dvs,dvs>& top_level_env);
void load_global(deviserstate* dstate, std::map<dvs,dvs>& top_level_env);

void store_global_var(deviserstate* dstate);
void load_global_var(deviserstate* dstate);
void store_global_func(deviserstate* dstate);
void load_global_func(deviserstate* dstate);

void push_constant(deviserstate* dstate, uint64_t constnum);

void dump_stack(deviserstate* dstate);
