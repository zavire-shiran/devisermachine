#include "deviser.hpp"

const int8_t load_var_op = 0;
const int8_t call_function_op = 1;
const int8_t return_function_op = 2;
const int8_t push_null_op = 3;

dvs run_bytecode(deviserstate* dstate);
void compile_function(deviserstate* dstate);