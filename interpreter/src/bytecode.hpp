#include "deviser.hpp"

const int8_t load_var_op = 0;
const int8_t call_function_op = 1;
const int8_t return_function_op = 2;
const int8_t push_null_op = 3;
const int8_t push_constant_op = 4;
const int8_t load_global_op = 5;
const int8_t load_global_func_op = 6;

dvs run_bytecode(deviserstate* dstate);
void compile_function(deviserstate* dstate);
