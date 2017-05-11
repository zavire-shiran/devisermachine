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
        call_function(dstate, 6);*/

        generate_lfunc(dstate);
        push_cfunc(dstate, testfunc);
        call_function(dstate, 1);
        run_bytecode(dstate);
        //dump_stack(dstate);

        destroy_deviser_state(dstate);

        return 0;
    } catch(const char* errormsg) {
        cout << errormsg << endl;
        return 1;
    }
}
