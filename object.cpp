#include "functional_object.h"

void Heap::CleanUp(Object* root) {
    used_.clear();
    for (Object* cur : objects_) {
        cur->Unmark();
    }
    Mark(root);
    std::vector<Object*> alive_objects;
    for (Object* cur : objects_) {
        if (cur != nullptr && !cur->Marked()) {
            delete cur;
        } else if (cur != nullptr) {
            alive_objects.push_back(cur);
        }
    }
    objects_ = alive_objects;
}

void Heap::Mark(Object* root) {
    if (!root) {
        return;
    }
    if (used_.count(root)) {
        return;
    }
    used_.insert(root);
    root->Mark();
    for (Object* nxt : root->Dep()) {
        Mark(nxt);
    }
}

Heap& Hp() {
    static Heap heap;
    return heap;
}

Object* Symbol::Eval(Object* scope) const {
    if (!Is<Scope>(scope)) {
        throw std::logic_error("Scope is not a scope in Symbol::Eval");
    }
    if (!As<Scope>(scope)->Exists(name_)) {
        throw NameError("can not eval symbol: no such name " + name_);
    }
    Object* me = As<Scope>(scope)->Find(name_);
    return me;
}

Object* Cell::Eval(Object* scope) const {
    if (!first_) {
        throw RuntimeError("cant recognize operator while evaluation");
    }
    Object* first_eval = first_->Eval(scope);
    if (!Is<FunctionalObject>(first_eval)) {
        throw RuntimeError("cant evaluate cell");
    }
    return As<FunctionalObject>(first_eval)->Calc(second_, scope);
}

std::string Cell::Serialize() const {
    std::string res = "(";
    Cell* cur = Hp().Make<Cell>(GetFirst(), GetSecond());
    while (true) {
        if (!cur->GetFirst()) {
            res += "()";
        } else {
            res += cur->GetFirst()->Serialize();
        }
        if (!cur->GetSecond()) {
            break;
        }
        res += " ";
        if (!Is<Cell>(cur->GetSecond())) {
            res += ". " + cur->GetSecond()->Serialize();
            break;
        }
        Cell* next = As<Cell>(cur->GetSecond());
        cur = next;
    }
    return res + ")";
}