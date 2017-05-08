#pragma once
#include <cstdint>

struct deviserstate;

// these need to be the same size as deviserobj*
typedef int64_t dvs_int;
typedef double dvs_float;

deviserstate* create_deviser_state();
void destroy_deviser_state(deviserstate*);

void rot_two(deviserstate* dstate);
void pop(deviserstate* dstate);
void dup(deviserstate* dstate);

void make_int(deviserstate* dstate, dvs_int value);
dvs_int get_int_value(deviserstate* dstate, uint64_t pos);

void make_cons(deviserstate* dstate);
void cons_car(deviserstate* dstate, uint64_t pos);
void cons_cdr(deviserstate* dstate, uint64_t pos);
