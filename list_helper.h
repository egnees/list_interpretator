#include "object.h"

std::vector<Object*> ObjectToList(Object*);

Object* ListToObject(const std::vector<Object*>&);

template <typename T>
std::vector<T*> EvalList(const std::vector<Object*>&, Object*);

template <typename T>
std::vector<T*> EvalList(const std::vector<Object*>& list, Object* scope) {
    std::vector<T*> ret;
    for (Object* cur : list) {
        if (!cur) {
            throw RuntimeError("list contains empty sublist");
        }
        Object* cur_eval = cur->Eval(scope);
        if (!Is<T>(cur_eval)) {
            throw RuntimeError("cant evaluate list");
        }
        ret.push_back(As<T>(cur_eval));
    }
    return ret;
}