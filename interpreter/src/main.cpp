#include "deviser.hpp"
#include "bytecode.hpp"
#include "gc.hpp"

#include <iostream>
#include <cstdint>
#include <sstream>

using std::cout;
using std::endl;
using std::string;

void testfunc(deviserstate* dstate);
void testfunc(deviserstate* dstate) {
    cout << "testfunc got " << workstacksize(dstate) << " arguments" << endl;
    push_null(dstate);
}

int main(int argc, char** argv) {
    string testmodule =
        "(module test\n"
        "  (defun f () 1)\n"
        "  (defmacro a () (quote asdf)))";

    try {
        deviserstate* dstate = create_deviser_state();

        std::shared_ptr<module_info> mod = get_module(dstate, "user");
        set_module(dstate, "user");
        read(dstate, "(defmacro a () (quote (quote asdf)))");
        eval(dstate);
        read(dstate, "(defun b () (quote asdf))");
        eval(dstate);

        read(dstate, "(defun c (&rest a) a)");
        eval(dstate);

        push_symbol(dstate, "car");
        push_cfunc(dstate, lisp_car, mod);
        store_module_func(dstate);

        push_symbol(dstate, "cdr");
        push_cfunc(dstate, lisp_cdr, mod);
        store_module_func(dstate);

        push_symbol(dstate, "cons");
        push_cfunc(dstate, lisp_cons, mod);
        store_module_func(dstate);

        push_symbol(dstate, "cons?");
        push_cfunc(dstate, lisp_consp, mod);
        store_module_func(dstate);

        push_symbol(dstate, "list");
        push_cfunc(dstate, lisp_list, mod);
        store_module_func(dstate);

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

/*        read(dstate, "testval");
        read(dstate, "2");
        store_module_var(dstate);

        read(dstate, "testfunc");
        push_cfunc(dstate, testfunc, mod);
        store_module_func(dstate);

        read(dstate, "(f (a) (if a (testfunc testval) (testfunc 1 2 3)))");
        compile_function(dstate, mod);
        print_lfunc_info(dstate);

        push_int(dstate, 1);
        call_function(dstate, 1);
        run_bytecode(dstate);
        print(dstate, cout);
        cout << endl;
        dump_stack(dstate);

        load_module(dstate, testmodule);*/

        while(std::cin) {
            std::cout << "> ";
            if(read(dstate, std::cin)) {
                run_gc(dstate);
                eval(dstate);
                print(dstate, std::cout);
                std::cout << std::endl;
                pop(dstate);
            }
        }
        std::cout << std::endl;

        destroy_deviser_state(dstate);

        return 0;
    } catch(const char* errormsg) {
        cout << errormsg << endl;
        return 1;
    }
}
