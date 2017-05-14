#include "deviser.hpp"
#include "bytecode.hpp"

#include <iostream>
#include <cstdint>
#include <sstream>

using std::cout;
using std::endl;
using std::string;

void testfunc(deviserstate* dstate);
void testfunc(deviserstate* dstate) {
    cout << "testfunc got " << workstacksize(dstate) << " arguments" << endl;
}

int main(int argc, char** argv) {
    try {
        deviserstate* dstate = create_deviser_state();

/*
        read(dstate, "a");
        store_variable(dstate, 0);
        load_variable(dstate, 0);
        load_variable(dstate, 0);
        load_variable(dstate, 0);
        read(dstate, "1");
        read(dstate, "2");
        read(dstate, "3");
        call_function(dstate, 6);
        std::vector<int8_t> bytecode = {load_var_op, 0,
                                        push_null_op,
                                        call_function_op, 1,
                                        return_function_op};
        generate_lfunc(dstate, 1, 1, bytecode);
        push_cfunc(dstate, testfunc);*/

        read(dstate, "(f () ())");
        compile_function(dstate);

        call_function(dstate, 0);
        run_bytecode(dstate);
        print(dstate, cout);
        cout << endl;
        //dump_stack(dstate);

        destroy_deviser_state(dstate);

        return 0;
    } catch(const char* errormsg) {
        cout << errormsg << endl;
        return 1;
    }
}
