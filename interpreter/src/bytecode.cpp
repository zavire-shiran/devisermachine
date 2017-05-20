#include "deviser.hpp"
#include "bytecode.hpp"

#include <iostream>

using std::vector;
using std::cout;
using std::endl;

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

        case push_constant_op:
        {
            size_t constnum = static_cast<uint64_t>(currentframe.bytecode[currentframe.pc+1]);
            currentframe.pc += 2;
            push_constant(dstate, constnum);
            break;
        }
        case load_global_op:
            load_global_var(dstate);
            currentframe.pc += 1;
            break;

        case load_global_func_op:
            load_global_func(dstate);
            currentframe.pc += 1;
            break;

        case branch_op:
            currentframe.pc = static_cast<uint64_t>(currentframe.bytecode[currentframe.pc+1]);
            break;

        case branch_if_null_op:
        {
            size_t jump_location = static_cast<uint64_t>(currentframe.bytecode[currentframe.pc+1]);
            currentframe.pc += 2;
            if(is_null(currentframe.workstack.back())) {
                currentframe.pc = jump_location;
            }
            pop(dstate);
            break;
        }

        default:
            throw "unknown opcode";
        }
    }
}

struct compilation_info {
    dvs name;
    vector<dvs> arguments;
    vector<dvs> variables;
    vector<dvs> constants;
    vector<int8_t> bytecode;
};

void extract_func_args(dvs func_args, compilation_info& cinfo);
int8_t get_variable_location(dvs var_name, compilation_info& cinfo);
int8_t add_const(dvs constant, compilation_info& cinfo);
void generate_if_statement(dvs statement, compilation_info& cinfo);
void generate_function_call(dvs statement, compilation_info& cinfo);
void generate_statement_bytecode(dvs statement, compilation_info& cinfo, bool function_position = false);

void extract_func_args(dvs func_args, compilation_info& cinfo) {
    while(!is_null(func_args)) {
        dvs arg = func_args->pcar();
        if(!is_symbol(arg)) {
            throw "arguments must be symbols";
        }
        cinfo.arguments.push_back(arg);
        func_args = func_args->cdr;
    }
}

int8_t get_variable_location(dvs var_name, compilation_info& cinfo) {
    for(uint32_t i = 0; i < cinfo.arguments.size(); ++i) {
        if(cinfo.arguments[i] == var_name) {
            return static_cast<int8_t>(i);
        }
    }

    for(uint32_t i = 0; i < cinfo.variables.size(); ++i) {
        if(cinfo.variables[i] == var_name) {
            return static_cast<int8_t>(i + cinfo.arguments.size());
        }
    }

    return -1;
}

int8_t add_const(dvs constant, compilation_info& cinfo) {
    int8_t constnum = static_cast<int8_t>(cinfo.constants.size());
    cinfo.constants.push_back(constant);
    return constnum;
}

void generate_if_statement(dvs statement, compilation_info& cinfo) {
    statement = statement->cdr;
    if(!is_cons(statement)) {
        throw "if requires condition clause";
    }
    dvs condition = statement->pcar();
    generate_statement_bytecode(condition, cinfo);
    cinfo.bytecode.push_back(branch_if_null_op);
    size_t branch_to_else_index = cinfo.bytecode.size();
    cinfo.bytecode.push_back(0);

    statement = statement->cdr;
    if(!is_cons(statement)) {
        throw "if requires true branch";
    }

    dvs true_branch = statement->pcar();
    generate_statement_bytecode(true_branch, cinfo);

    statement = statement->cdr;
    if(is_cons(statement)) {
        cinfo.bytecode.push_back(branch_op);
        size_t branch_to_end_index = cinfo.bytecode.size();
        cinfo.bytecode.push_back(0);
        cinfo.bytecode[branch_to_else_index] = static_cast<int8_t>(cinfo.bytecode.size());
        generate_statement_bytecode(statement->pcar(), cinfo);
        cinfo.bytecode[branch_to_end_index] = static_cast<int8_t>(cinfo.bytecode.size());
    } else {
        cinfo.bytecode[branch_to_else_index] = static_cast<int8_t>(cinfo.bytecode.size());
    }
}

void generate_function_call(dvs statement, compilation_info& cinfo) {
    int8_t argc = 0;
    generate_statement_bytecode(statement->pcar(), cinfo, true);
    statement = statement->cdr;
    while(is_cons(statement)) {
        ++argc;
        generate_statement_bytecode(statement->pcar(), cinfo);
        statement = statement->cdr;
    }
    if(!is_null(statement)) {
        throw "invalid function call";
    }
    cinfo.bytecode.push_back(call_function_op);
    cinfo.bytecode.push_back(argc);
}

void generate_statement_bytecode(dvs statement, compilation_info& cinfo, bool function_position) {
    if(is_null(statement)) {
        cinfo.bytecode.push_back(push_null_op);
    } else if(is_symbol(statement)) {
        if(function_position) {
            // eventually we will need to look in local funcs, but we don't have that now
            int8_t constnum = add_const(statement, cinfo);
            cinfo.bytecode.push_back(push_constant_op);
            cinfo.bytecode.push_back(constnum);
            cinfo.bytecode.push_back(load_global_func_op);
        } else {
            int8_t local_varnum = get_variable_location(statement, cinfo);
            if(local_varnum >= 0) {
                cinfo.bytecode.push_back(load_var_op);
                cinfo.bytecode.push_back(local_varnum);
            } else {
                int8_t constnum = add_const(statement, cinfo);
                cinfo.bytecode.push_back(push_constant_op);
                cinfo.bytecode.push_back(constnum);
                cinfo.bytecode.push_back(load_global_op);
            }
        }
    } else if(is_int(statement)) {
        int8_t constnum = add_const(statement, cinfo);
        cinfo.bytecode.push_back(push_constant_op);
        cinfo.bytecode.push_back(constnum);
    } else if(is_cons(statement)) {
        dvs first_element = statement->pcar();
        if(is_symbol(first_element) && symbol_string(first_element) == "if") {
            generate_if_statement(statement, cinfo);
        } else {
            generate_function_call(statement, cinfo);
        }
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
                   cinfo.constants,
                   cinfo.bytecode);
}

void disassemble_bytecode(vector<int8_t> bytecode, std::ostream& out) {
    size_t index = 0;
    while(index < bytecode.size()) {
        int8_t command_code = bytecode[index];
        switch(command_code) {
        case load_var_op:
            out << index << " load_var_op " << static_cast<int>(bytecode[index+1]) << endl;
            index += 2;
            break;

        case call_function_op:
            out << index << " call_function_op " << static_cast<int>(bytecode[index+1]) << endl;
            index += 2;
            break;

        case return_function_op:
            out << index << " return_function_op" << endl;
            index += 1;
            break;

        case push_null_op:
            out << index << " push_null_op" << endl;
            index += 1;
            break;

        case push_constant_op:
            out << index << " push_constant_op " << static_cast<int>(bytecode[index+1]) << endl;
            index += 2;
            break;

        case load_global_op:
            out << index << " load_global_op" << endl;
            index += 1;
            break;

        case load_global_func_op:
            out << index << " load_global_func_op" << endl;
            index += 1;
            break;

        case branch_op:
            out << index << " branch_op " << static_cast<int>(bytecode[index+1]) << endl;
            index += 2;
            break;

        case branch_if_null_op:
            out << index << " branch_if_null_op " << static_cast<int>(bytecode[index+1]) << endl;
            index += 2;
            break;
        }
    }
}
