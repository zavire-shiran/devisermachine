#include "deviser.hpp"

#include <iostream>
#include <cstdint>

using std::cout;
using std::endl;
using std::string;

int main(int argc, char** argv) {
    deviserstate* dstate = create_deviser_state();
    push_int(dstate, 0);
    push_int(dstate, 200);
    push_null(dstate);
    make_cons(dstate);
    make_cons(dstate);

    print(dstate, cout);
    cout << endl;

    push_symbol(dstate, "testsym");
    string name = get_symbol_name(dstate, 0);
    cout << "symbol name: " << name << endl;
    destroy_deviser_state(dstate);

    return 0;
}
