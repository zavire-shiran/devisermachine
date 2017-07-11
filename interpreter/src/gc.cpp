#include "gc.hpp"

#include "deviser.hpp"

#include <deque>
#include <vector>

using std::deque;
using std::vector;
using std::map;

void mark(dvs item, deque<dvs>& markingqueue);
void mark(vector<dvs> items, deque<dvs>& markingqueue);
void mark(std::shared_ptr<module_info> mod, deque<dvs>& markingqueue);
void mark(map<dvs, dvs> bindings, deque<dvs>& markingqueue);

void mark(dvs item, deque<dvs>& markingqueue) {
    if(!is_marked(item)) {
        mark(item);
        markingqueue.push_back(item);
    }
}

void mark(vector<dvs> items, deque<dvs>& markingqueue) {
    for(auto item : items) {
        mark(item, markingqueue);
    }
}

void mark(std::shared_ptr<module_info> mod, deque<dvs>& markingqueue) {
    mark(mod->name, markingqueue);
    mark(mod->value_bindings, markingqueue);
    mark(mod->func_bindings, markingqueue);
}

void mark(map<dvs, dvs> bindings, deque<dvs>& markingqueue) {
    for(auto binding : bindings) {
        mark(binding.first, markingqueue);
        mark(binding.second, markingqueue);
    }
}

void run_gc(deviserstate* dstate) {
    //run through all of the dvs things referenced in dstate
    //for each, mark, then put into a queue to mark held references
    deque<dvs> markingqueue;

    for(auto stackframe : dstate->stack) {
        mark(stackframe.workstack, markingqueue);
        mark(stackframe.variables, markingqueue);
        mark(stackframe.constants, markingqueue);
    }

    for(auto symbol_entry : dstate->symbol_table) {
        mark(symbol_entry.second, markingqueue);
    }

    for(auto module_entry : dstate->modules) {
        // is this necessary? could the module be named by an uninterned symbol?
        // it would get caught in the module_info anyway
        // ...as long as those get updated together
        mark(module_entry.first, markingqueue);
        mark(module_entry.second, markingqueue);
    }

    //then run through queue
    //for each, mark its references, then put those in the queue
    //(don't put anything already marked in the queue)
    //continue until queue exhausted
    while(!markingqueue.empty()) {
        dvs item = markingqueue.front();
        switch(get_typeid(item)) {
        case cons_typeid:
            mark(item->pcar(), markingqueue);
            mark(item->cdr, markingqueue);
            break;

        case cfunc_typeid:
        {
            cfunc_info* finfo = reinterpret_cast<cfunc_info*>(item->cdr);
            mark(finfo->module, markingqueue);
            break;
        }
        case lfunc_typeid:
        case macro_typeid:
        {
            lfunc_info* finfo = reinterpret_cast<lfunc_info*>(item->cdr);
            mark(finfo->name, markingqueue);
            mark(finfo->constants, markingqueue);
            mark(finfo->module, markingqueue);
            break;
        }
        case module_typeid:
            break;
        case null_typeid:
            break;
        case int_typeid:
            break;
        case symbol_typeid:
            break;
        default:
            throw "unknown typeid";
        }
    }

    //iterate through memory arenas
    //for each not already freed cell:
    //  if not marked, clear and put into free list
    //  else just clear the mark
    for(auto arena : dstate->memoryarenas) {
        for(size_t i = 0; i < arena.size; ++i) {
            dvs item = arena.arena + i;
            if(!is_free(item)) {
                if(is_marked(item)) {
                    unmark(item);
                } else {
                    free(item);
                    dstate->freelist.push_front(item);
                }
            }
        }
    }
}
