#include <histedit.h>
#include <string>

class LineEditor {
public:
    LineEditor(std::string progname);
    ~LineEditor() = default;

    LineEditor(LineEditor&&) = default;
    LineEditor& operator=(LineEditor&&) & = default;

    LineEditor(const LineEditor&) = delete;
    LineEditor& operator=(const LineEditor&) & = delete;

    std::string getLine();
    void setError();

private:
    std::unique_ptr<EditLine, void (*)(EditLine*)> editState;
    bool isError;
};
