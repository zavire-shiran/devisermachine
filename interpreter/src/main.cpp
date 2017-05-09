#include "deviser.hpp"

#include <iostream>
#include <cstdint>
#include <sstream>

using std::cout;
using std::endl;
using std::string;

void test_read(deviserstate* dstate, string test) {
    std::stringstream ss(test);
    read(dstate, ss);
    print(dstate, cout);
    pop(dstate);
    cout << endl;
}

int main(int argc, char** argv) {
    deviserstate* dstate = create_deviser_state();
    push_symbol(dstate, "test");
    push_int(dstate, 200);
    push_null(dstate);
    make_cons(dstate);
    make_cons(dstate);

    print(dstate, cout);
    cout << endl;

    test_read(dstate, "11");
    test_read(dstate, "test");
    test_read(dstate, "()");
    test_read(dstate, "(test)");
    test_read(dstate, "(aa bb cc dd)");
    test_read(dstate, "(test 11)");
    test_read(dstate, "(test a 123)");
    dump_stack(dstate);

    destroy_deviser_state(dstate);

    return 0;
}
