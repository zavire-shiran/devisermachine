#pragma once

#include <cstdint>

struct deviserobj;
typedef deviserobj* dvs;

struct deviserstate;
struct module_info;

// these need to be the same size as deviserobj*
typedef int64_t dvs_int;
typedef double dvs_float;
typedef void (*cfunc_type)(deviserstate*);

typedef int16_t bytecode;

