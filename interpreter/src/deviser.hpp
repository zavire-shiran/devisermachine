#pragma once
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>
#include <map>

struct deviserobj;
typedef deviserobj* dvs;

struct deviserstate;

// these need to be the same size as deviserobj*
typedef int64_t dvs_int;
typedef double dvs_float;
typedef void (*cfunc_type)(deviserstate*);

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
    std::vector<int8_t> bytecode;
    uint64_t pc;
};

struct deviserstate {
    dvs memoryarena;
    size_t memoryarenasize;
    size_t nextfree;
    std::vector<stackframe> stack;
    std::map<std::string, dvs> symbol_table;
};

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

void generate_lfunc(deviserstate* dstate);

void store_variable(deviserstate* dstate, uint64_t varnum);
void load_variable(deviserstate* dstate, uint64_t varnum);

void dump_stack(deviserstate* dstate);
