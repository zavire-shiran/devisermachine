#include "deviser.hpp"

#include <vector>
#include <iostream>

using std::vector;

struct deviserobj;
typedef deviserobj* dvs;

dvs alloc_dvs(deviserstate* dstate);

bool is_null(dvs d);
bool is_cons(dvs d);
bool is_marked(dvs d);

void set_typeid(dvs d, dvs_int tid);
dvs_int get_typeid(dvs d);

bool is_int(dvs d);
dvs_int get_int(dvs d);

const dvs_int int_typeid = 1;

struct deviserobj {
    dvs pcar() {
        // clear the low 2 bits
        return reinterpret_cast<dvs>(reinterpret_cast<dvs_int>(car) & (~0x3));
    }
    dvs car;
    dvs cdr;
};

struct deviserstate {
    dvs memoryarena;
    size_t memoryarenasize;
    size_t nextfree;
    vector<dvs> stack;
};

deviserstate* create_deviser_state() {
    deviserstate* dstate = new deviserstate;
    dstate->memoryarenasize = 1000;
    dstate->nextfree = 0;
    dstate->memoryarena = new deviserobj[dstate->memoryarenasize];
    return dstate;
}

dvs alloc_dvs(deviserstate* dstate) {
    if(dstate->nextfree < dstate->memoryarenasize) {
        dvs newalloc = dstate->memoryarena + dstate->nextfree++;
        dstate->stack.push_back(newalloc);
        return newalloc;
    } else {
        return nullptr;
    }
}

void destroy_deviser_state(deviserstate* ds) {
    delete ds;
}

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

dvs_int get_int(dvs d) {
    if(is_int(d)) {
        return reinterpret_cast<dvs_int>(d->cdr);
    } else {
        throw "not an integer";
    }
}

void make_int(deviserstate* dstate, dvs_int value) {
    dvs newint = alloc_dvs(dstate);
    set_typeid(newint, int_typeid);
    newint->cdr = reinterpret_cast<dvs>(value);
}

dvs_int get_int_value(deviserstate* dstate, uint64_t pos) {
    uint64_t int_pos = dstate->stack.size() - (pos + 1);
    if(int_pos >= dstate->stack.size()) {
        throw "stack pos out of range";
    }
    dvs i = dstate->stack[int_pos];
    return get_int(i);
}
