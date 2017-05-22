#pragma once

#include "deviser.hpp"
#include <ostream>

const bytecode load_var_op = 0;
const bytecode call_function_op = 1;
const bytecode return_function_op = 2;
const bytecode push_null_op = 3;
const bytecode push_constant_op = 4;
const bytecode load_global_op = 5;
const bytecode load_global_func_op = 6;
const bytecode branch_op = 7;
const bytecode branch_if_null_op = 8;

dvs run_bytecode(deviserstate* dstate);
void compile_function(deviserstate* dstate);
void disassemble_bytecode(std::vector<bytecode> bytecode, std::ostream& out);
