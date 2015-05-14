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
    bool isEndOfFile() const;

private:
    std::unique_ptr<EditLine, void (*)(EditLine*)> editstate;
    bool iserror;
    bool endoffile;
};
