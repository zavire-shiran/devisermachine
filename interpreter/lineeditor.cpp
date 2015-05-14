#include "lineeditor.hpp"
#include <cstdio>

using std::string;

LineEditor::LineEditor(std::string progname) :
    editstate(el_init(progname.c_str(), stdin, stdout, stderr),
              [](EditLine* el) { el_end(el); }),
    iserror(false),
    endoffile(false)
{
}

string LineEditor::getLine() {
    int count;
    const char* line_read = el_gets(editstate.get(), &count);

    if(line_read == nullptr) {
        endoffile = true;
        return "";
    }

    string line(line_read);


    if(count == -1) {
        setError();
    }

    return line;
}

void LineEditor::setError() {
    iserror = true;
}

bool LineEditor::isEndOfFile() const {
    return endoffile;
}
