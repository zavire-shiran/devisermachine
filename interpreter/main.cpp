#include <memory>
#include <string>
#include <istream>
#include <iostream>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <unistd.h>

#include "lineeditor.hpp"
#include "deviser.hpp"

using std::shared_ptr;
using std::cout;
using std::endl;
using std::dynamic_pointer_cast;
using std::make_shared;

bool ismodulecommand(shared_ptr<lispobj> lobj) {
    if(lobj->objtype() != CONS_TYPE) {
        return false;
    }

    shared_ptr<cons> c = dynamic_pointer_cast<cons>(lobj);
    if(c->car()->objtype() != SYMBOL_TYPE) {
        return false;
    }

    shared_ptr<symbol> sym = dynamic_pointer_cast<symbol>(c->car());
    string formname = sym->name();
    return formname == "define" || formname == "import" ||
        formname == "export" || formname == "init" ||
        formname == "undefine" || formname == "unimport" ||
        formname == "unexport";
}

void read_eval_print(const string& lispstr, shared_ptr<module> mod) {
    //cout << "(print (eval (read \"" << lispstr << "\"))) => ";
    auto lobj = read(lispstr);
    auto retlobj = mod->eval(lobj);
    print(retlobj);
    cout << endl;
}

vector< shared_ptr<lispobj> > read_file(string filename) {
    std::ifstream infile(filename);
    string file_contents;
    vector< shared_ptr<lispobj> > ret;
    if(!infile) {
        cout << "Couldn't open " << filename << endl;
        return ret;
    }

    infile.seekg(0, std::ios::end);
    file_contents.resize(infile.tellg());
    infile.seekg(0, std::ios::beg);
    infile.read(&file_contents[0], file_contents.size());

    ret = readall(file_contents);

    return ret;
}

void usage() {
    cout << "usage message XD" << endl;
}

vector<string> find_modules(string module_path) {
    vector<string> ret;

    char* dirs[2];
    dirs[0] = (char*)calloc(module_path.size() + 1, sizeof(char));
    module_path.copy(dirs[0], module_path.size());
    dirs[1] = NULL;
    FTS* module_dir = fts_open(dirs, FTS_COMFOLLOW | FTS_LOGICAL, NULL);
    FTSENT* file_ent;

    if(module_dir == NULL) {
        cout << "error opening module path" << endl;
        return ret;
    }

    while((file_ent = fts_read(module_dir)) != NULL) {
        // this switch should probably do some error detection
        switch(file_ent->fts_info) {
        case FTS_F:
            ret.push_back(file_ent->fts_path);
            break;
        }
    }

    return ret;
}

shared_ptr<lispobj> eval_file(string filename, shared_ptr<module> mod) {
    vector< shared_ptr<lispobj> > v = read_file(filename);
    shared_ptr<lispobj> code(new cons(std::make_shared<symbol>("begin"),
                                      make_list(v.begin(), v.end())));
    return eval(code, mod->get_bindings());
}

int main(int argc, char** argv)
{
    int ch;
    string filesystem = ".";

    while((ch = getopt(argc, argv, "hf:")) != -1) {
        switch(ch) {
        case 'f':
            filesystem = optarg;
            break;
        case 'h':
        default:
            usage();
            return 1;
        }
    }
    argc -= optind;
    argv += optind;

    shared_ptr<module> builtins_module = make_builtins_module();
    shared_ptr<module> user_module(new module(make_shared<cons>(make_shared<symbol>("user"),
                                                                make_shared<nil>())));
    user_module->add_import(builtins_module->get_name());

    LineEditor ledit("deviser");
    string input = ledit.getLine();
    while(!ledit.isEndOfFile()) {
        read_eval_print(input, user_module);
        input = ledit.getLine();
    }
    cout << endl;
}
