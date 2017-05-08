#include "deviser.hpp"

#include <iostream>
#include <cstdint>

using std::cout;
using std::endl;

int main(int argc, char** argv) {
    deviserstate* dstate = create_deviser_state();
    make_int(dstate, 0);
    make_int(dstate, 200);
    make_cons(dstate);
    cons_car(dstate, 0);
    cons_cdr(dstate, 1);

    cout << "top int: " << get_int_value(dstate, 0) <<
        " next int: " << get_int_value(dstate, 1) << endl;
    destroy_deviser_state(dstate);

    return 0;
}
