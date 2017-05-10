#include "deviser.hpp"

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

        push_cfunc(dstate, testfunc);
        read(dstate, "a");
        read(dstate, "b");
        read(dstate, "c");
        read(dstate, "1");
        read(dstate, "2");
        read(dstate, "3");
        call_function(dstate, 6);
        dump_stack(dstate);

        destroy_deviser_state(dstate);

        return 0;
    } catch(const char* errormsg) {
        cout << errormsg << endl;
        return 1;
    }
}
