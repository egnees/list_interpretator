#include "list_helper.h"

std::vector<Object*> ObjectToList(Object* o) {
    std::vector<Object*> ret;
    if (!o) {
        return ret;
    }
    if (!Is<Cell>(o)) {
        throw RuntimeError("list helper: object to list: root must be a cell");
    }
    while (true) {
        ret.push_back(As<Cell>(o)->GetFirst());
        if (!As<Cell>(o)->GetSecond()) {
            break;
        }
        if (!Is<Cell>(As<Cell>(o)->GetSecond())) {
            ret.push_back(As<Cell>(o)->GetSecond());
            break;
        }
        o = As<Cell>(o)->GetSecond();
    }
    return ret;
}

Object* ListToObject(const std::vector<Object*>& list) {
    if (list.empty()) {
        return nullptr;
    }
    Cell* first_cell_ptr = Hp().Make<Cell>(nullptr);
    Cell* current_cell_ptr(first_cell_ptr);
    for (Object* sp : list) {
        if (current_cell_ptr->GetFirst()) {
            Cell* next_cell_ptr = Hp().Make<Cell>(nullptr);
            current_cell_ptr->SetSecond(next_cell_ptr);
            current_cell_ptr = next_cell_ptr;
        }
        current_cell_ptr->SetFirst(sp);
    }
    return first_cell_ptr;
}