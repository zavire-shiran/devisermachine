#include "deviser.hpp"
#include "bytecode.hpp"

dvs run_bytecode(deviserstate* dstate) {
    while(true) {
        stackframe& currentframe = dstate->stack.back();
        switch(currentframe.bytecode[currentframe.pc]) {
        case load_var_op:
        {
            uint64_t varnum = static_cast<uint64_t>(currentframe.bytecode[currentframe.pc+1]);
            load_variable(dstate, varnum);
            currentframe.pc += 2;
            break;
        }
        case call_function_op:
        {
            uint64_t argc = static_cast<uint64_t>(currentframe.bytecode[currentframe.pc+1]);
            currentframe.pc += 2;
            call_function(dstate, argc);
            break;
        }
        case return_function_op:
        {
            return_function(dstate);
            if(dstate->stack.size() == 1) {
                return dstate->stack.back().workstack.back();
            }
            break;
        }
        case push_null_op:
            push_null(dstate);
            currentframe.pc += 1;
            break;

        default:
            throw "unknown opcode";
        }
    }
}
