#include "console.hpp"
#include <curses.h>

using std::shared_ptr;
using std::make_shared;

shared_ptr<lispobj> console_init(vector< shared_ptr<lispobj> > /*args*/) {
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    printw("Hello World !!!");
    refresh();
    getch();

    return make_shared<nil>();
}

shared_ptr<lispobj> console_end(vector< shared_ptr<lispobj> > /*args*/) {
    endwin();

    return make_shared<nil>();
}

shared_ptr<module> make_console_module(shared_ptr<lexicalscope> top_level_scope) {
    shared_ptr<lispobj> module_name(new cons(make_shared<symbol>("console"),
                                             make_shared<nil>()));
    shared_ptr<module> console_module(new module(module_name, top_level_scope));

    console_module->defun_and_export("console-init", make_shared<cfunc>(console_init));
    console_module->defun_and_export("console-end", make_shared<cfunc>(console_end));

    return console_module;
}
