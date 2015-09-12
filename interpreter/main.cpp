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

void read_eval_print(const string& lispstr, shared_ptr<module> mod) {
    //cout << "(print (eval (read \"" << lispstr << "\"))) => ";
    auto lobj = read(lispstr);
    //lobj->print(); cout << endl;
    auto retlobj = mod->eval(lobj);
    retlobj->print();
    cout << endl;
}

vector< shared_ptr<lispobj> > read_file(string filename) {
    shared_ptr<std::ifstream> infile(new std::ifstream(filename));
    vector< shared_ptr<lispobj> > ret;
    if(!(infile->good())) {
        cout << "Couldn't open " << filename << endl;
        return ret;
    }

    reader r(std::static_pointer_cast<std::istream>(infile), filename);

    ret = r.readall();

    return ret;
}

void usage() {
    cout << "usage message XD" << endl;
}

bool ends_with_tilde(const string s) {
    return *(s.rbegin()) == '~';
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
            if(!ends_with_tilde(file_ent->fts_path)) {
                ret.push_back(file_ent->fts_path);
            }
            break;
        }
    }

    return ret;
}

vector< shared_ptr<lispobj> > eval_file(string filename, shared_ptr<lexicalscope> scope) {
    vector< shared_ptr<lispobj> > file_forms = read_file(filename);
    vector< shared_ptr<lispobj> > return_values;

    for(auto form : file_forms) {
        return_values.push_back(eval(form, scope));
    }

    return return_values;
}

int main(int argc, char** argv)
{
    int ch;
    vector<string> modulesdirs{"../kernel-modules", "../compiler"};
    vector<string> statements_to_run;

    while((ch = getopt(argc, argv, "e:hm:")) != -1) {
        switch(ch) {
        case 'e':
            statements_to_run.push_back(optarg);
            break;
        case 'm':
            modulesdirs.push_back(optarg);
            break;
        case 'h':
        default:
            usage();
            return 1;
        }
    }
    argc -= optind;
    argv += optind;

    vector<string> modules_to_load;
    shared_ptr<lexicalscope> top_level_scope(new lexicalscope);

    for(auto directory : modulesdirs) {
        vector<string> modules_found = find_modules(directory);
        modules_to_load.insert(modules_to_load.end(), modules_found.begin(), modules_found.end());
    }

    shared_ptr<module> builtins_module = make_builtins_module(top_level_scope);
    top_level_scope->add_import(builtins_module);

    shared_ptr<module> user_module(new module(make_shared<cons>(make_shared<symbol>("user"),
                                                                make_shared<nil>()),
                                              top_level_scope));

    for(auto module_file : modules_to_load) {
        cout << "loading " << module_file << endl;
        eval_file(module_file, top_level_scope);
    }

    if(!statements_to_run.empty()) {
        for(auto statement : statements_to_run) {
            read_eval_print(statement, user_module);
        }
    } else {
        LineEditor ledit("deviser");
        string input = ledit.getLine();
        while(!ledit.isEndOfFile()) {
            read_eval_print(input, user_module);
            input = ledit.getLine();
        }
        cout << endl;
    }

    return 0;
}
