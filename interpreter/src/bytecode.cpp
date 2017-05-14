#include "deviser.hpp"
#include "bytecode.hpp"

using std::vector;

vector<dvs> extract_func_args(dvs func_args);

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

struct compilation_info {
    dvs name;
    vector<dvs> arguments;
    vector<dvs> variables;
    vector<int8_t> bytecode;
};

void extract_func_args(dvs func_args, compilation_info& cinfo);
void generate_statement_bytecode(dvs statement, compilation_info& cinfo);

void extract_func_args(dvs func_args, compilation_info& cinfo) {
    while(!is_null(func_args)) {
        dvs arg = func_args->pcar();
        if(!is_symbol(arg)) {
            throw "arguments must be symbols";
        }
        cinfo.arguments.push_back(arg);
    }
}

void generate_statement_bytecode(dvs statement, compilation_info& cinfo) {
    if(is_null(statement)) {
        cinfo.bytecode.push_back(push_null_op);
    } else {
        throw "cannot compile statement";
    }
}

void compile_function(deviserstate* dstate) {
    stackframe& currentframe = dstate->stack.back();
    dvs func_sexp = currentframe.workstack.back();
    compilation_info cinfo;

    if(!is_cons(func_sexp)) {
        throw "only cons can be compiled";
    }

    cinfo.name = func_sexp->pcar();
    if(!is_symbol(cinfo.name)) {
        throw "function name must be a symbol";
    }

    func_sexp = func_sexp->cdr;
    if(!is_cons(func_sexp)) {
        throw "function must have arguments";
    }

    dvs func_args = func_sexp->pcar();
    if(!is_list(func_args)) {
        throw "function arguments must be list";
    }

    extract_func_args(func_args, cinfo);

    func_sexp = func_sexp->cdr;
    while(!is_null(func_sexp)) {
        if(!is_cons(func_sexp)) {
            throw "improper list in function declaration";
        }
        generate_statement_bytecode(func_sexp->pcar(), cinfo);
        func_sexp = func_sexp->cdr;
    }

    if(cinfo.bytecode.empty()) {
        cinfo.bytecode.push_back(push_null_op);
    }

    cinfo.bytecode.push_back(return_function_op);

    generate_lfunc(dstate, cinfo.arguments.size(),
                   cinfo.arguments.size() + cinfo.variables.size(),
                   cinfo.bytecode);
}
