#include "deviser.hpp"

lispobj::lispobj() {}

symbol::symbol(string sn) {
    symname = sn;
}

int symbol::objtype() const {
    return SYMBOL_TYPE;
}

string symbol::name() const {
    return symname;
}

cons::cons(shared_ptr<lispobj> a, shared_ptr<lispobj> d) {
    first = a;
    second = d;
}

int cons::objtype() const {
    return CONS_TYPE;
}

shared_ptr<lispobj> cons::car() const {
    return first;
}

shared_ptr<lispobj> cons::cdr() const {
    return second;
}

number::number(int n) {
    num = n;
}

int number::objtype() const {
    return NUMBER_TYPE;
}

int number::value() const {
    return num;
}

bool eq(shared_ptr<lispobj> left, shared_ptr<lispobj> right) {
    if(left == right) return true;
    if(left->objtype() != right->objtype()) return false;

    switch(left->objtype()) {
    case SYMBOL_TYPE:
    {
	shared_ptr<symbol> ls(std::dynamic_pointer_cast<symbol>(left));
	shared_ptr<symbol> rs(std::dynamic_pointer_cast<symbol>(right));
	return ls->name() == rs->name();
    }

    case NIL_TYPE:
	return true;

    default:
	return false;
    };
}

bool eqv(shared_ptr<lispobj> left, shared_ptr<lispobj> right) {
    if(eq(left, right)) return true;
    if(left->objtype() != right->objtype()) return false;

    switch(left->objtype()) {
    case NUMBER_TYPE:
    {
	shared_ptr<number> ln(std::dynamic_pointer_cast<number>(left));
	shared_ptr<number> rn(std::dynamic_pointer_cast<number>(right));
	return ln->value() && rn->value();
    }

    default:
	return false;
    };
}

bool equal(shared_ptr<lispobj> left, shared_ptr<lispobj> right) {
    if(eqv(left, right)) return true;
    if(left->objtype() != right->objtype()) return false;

    switch(left->objtype()) {
    case CONS_TYPE:
    {
	shared_ptr<cons> lc(std::dynamic_pointer_cast<cons>(left));
	shared_ptr<cons> rc(std::dynamic_pointer_cast<cons>(right));
	return equal(lc->car(), rc->car()) && equal(lc->cdr(), rc->cdr());
    }
    
    default:
	return false;
    };
}
