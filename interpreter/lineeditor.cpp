#include "lineeditor.hpp"
#include <cstdio>

using std::string;

LineEditor::LineEditor(std::string progname) :
    editState(el_init(progname.c_str(), stdin, stdout, stderr),
              [](EditLine* el) { el_end(el); }),
    isError(false)
{
}

string LineEditor::getLine() {
    int count;
    string line(el_gets(editState.get(), &count));

    if(count == -1) {
        setError();
    }

    return line;
}

void LineEditor::setError() {
    isError = true;
}

