#include <memory>
#include <string>
#include <istream>
#include <iostream>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <unistd.h>

#include "deviser.hpp"

using std::shared_ptr;
using std::cout;
using std::endl;

void read_eval_print(const string& lispstr, shared_ptr<environment> env) {
    cout << "(print (eval (read \"" << lispstr << "\"))) => ";
    print(eval(read(lispstr), env->get_scope(), env));
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
        switch(file_ent->fts_info) {
        case FTS_F:
            ret.push_back(file_ent->fts_path);
            break;
        }
    }

    return ret;
}

int main(int argc, char** argv)
{
    int ch;
    string module_path = "../modules";
    string execute;
    vector<string> modules_to_load;

    while((ch = getopt(argc, argv, "he:m:")) != -1) {
        switch(ch) {
        case 'e':
            execute = optarg;
            break;
        case 'm':
            module_path = optarg;
            break;
        case 'h':
        default:
            usage();
            return 1;
        }
    }

    shared_ptr<environment> env = make_standard_env();

    modules_to_load = find_modules(module_path);
    for(auto mod_file : modules_to_load) {
        if(mod_file.substr(mod_file.size() - 4) == ".dvs") {
            cout << "Read " << mod_file << endl;
            vector< shared_ptr<lispobj> > v = read_file(mod_file);
            shared_ptr<lispobj> code(new cons(std::make_shared<symbol>("begin"),
                                              make_list(v)));
            eval(code, env->get_scope(), env);
        }
    }
}
