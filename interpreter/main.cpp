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
        // this switch should probably do some error detection
        switch(file_ent->fts_info) {
        case FTS_F:
            ret.push_back(file_ent->fts_path);
            break;
        }
    }

    return ret;
}

shared_ptr<lispobj> eval_file(string filename, shared_ptr<environment> env) {
    vector< shared_ptr<lispobj> > v = read_file(filename);
    shared_ptr<lispobj> code(new cons(std::make_shared<symbol>("begin"),
                                      make_list(v)));
    return eval(code, env->get_scope(), env);
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

    shared_ptr<environment> env = make_standard_env();

    LineEditor ledit("deviser");
    string input = ledit.getLine();
    cout << input;
}
