#pragma once
#include <cstdint>

struct deviserstate;

// these need to be the same size as deviserobj*
typedef int64_t dvs_int;
typedef double dvs_float;

deviserstate* create_deviser_state();
void destroy_deviser_state(deviserstate*);

void make_int(deviserstate* dstate, dvs_int value);
dvs_int get_int_value(deviserstate* dstate, uint64_t pos);
