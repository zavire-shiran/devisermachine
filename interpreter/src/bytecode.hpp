#include "deviser.hpp"
#include <ostream>

const int8_t load_var_op = 0;
const int8_t call_function_op = 1;
const int8_t return_function_op = 2;
const int8_t push_null_op = 3;
const int8_t push_constant_op = 4;
const int8_t load_global_op = 5;
const int8_t load_global_func_op = 6;
const int8_t branch_op = 7;
const int8_t branch_if_null_op = 8;

dvs run_bytecode(deviserstate* dstate);
void compile_function(deviserstate* dstate);
void disassemble_bytecode(std::vector<int8_t> bytecode, std::ostream& out);
