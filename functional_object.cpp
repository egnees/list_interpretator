#include "functional_object.h"
#include "list_helper.h"

Object* FunctionalObject::Calc(Object* o, Object* scope) const {
    return Calc(ObjectToList(o), scope);
}

Object* QuoteFunctor::Calc(const std::vector<Object*>& list, Object*) const {
    if (list.size() != 1) {
        throw RuntimeError("quote needs exactly one argument");
    }
    return list[0];
}

Object* BooleanFunctor::Calc(const std::vector<Object*>& list, Object* scope) const {
    bool ret = !stop_value_;
    if (list.empty()) {
        return Hp().Make<Boolean>(ret);
    }
    for (Object* cur : list) {
        if (!cur) {
            throw RuntimeError("list contains empty sublist");
        }
        Object* cur_eval = cur->Eval(scope);
        bool converted_to_bool = !Is<Boolean>(cur_eval) || As<Boolean>(cur_eval)->GetValue();
        ret = functor_(ret, converted_to_bool);
        if (ret == stop_value_) {
            return cur_eval;
        }
    }
    return list.back()->Eval(scope);
}

Object* NumberFunctor::Calc(const std::vector<Object*>& list, Object* scope) const {
    int64_t ret;
    if (first_.has_value()) {
        ret = first_.value();
    } else if (list.empty()) {
        throw RuntimeError(
            "number functions without first defined value can not be computed by zero values");
    }
    bool first_value = true;
    for (Object* cur : list) {
        if (!cur) {
            throw RuntimeError("list contains empty sublist");
        }
        Object* cur_eval = cur->Eval(scope);
        if (!Is<Number>(cur_eval)) {
            throw RuntimeError("number function argument must be numbers");
        }
        if (first_value) {
            ret = As<Number>(cur_eval)->GetValue();
            first_value = false;
        } else {
            ret = functor_(ret, As<Number>(cur_eval)->GetValue());
        }
    }
    return Hp().Make<Number>(ret);
}

Object* BooleanNot::Calc(const std::vector<Object*>& list, Object* scope) const {
    if (list.size() != 1) {
        throw RuntimeError("not operator works with 1-element list only");
    }
    Object* eval = list.back()->Eval(scope);
    bool res = Is<Boolean>(eval) && !As<Boolean>(eval)->GetValue();
    return Hp().Make<Boolean>(res);
}

Object* CompareFunctor::Calc(const std::vector<Object*>& list, Object* scope) const {
    auto list_eval = EvalList<Number>(list, scope);
    for (size_t i = 0; i + 1 < list_eval.size(); ++i) {
        if (!functor_(list_eval[i]->GetValue(), list_eval[i + 1]->GetValue())) {
            return Hp().Make<Boolean>(false);
        }
    }
    return Hp().Make<Boolean>(true);
}

Object* NumberAbs::Calc(const std::vector<Object*>& list, Object* scope) const {
    if (list.size() != 1) {
        throw RuntimeError("abs operator works with 1-element list only");
    }
    Object* elem_eval = list.back()->Eval(scope);
    if (!Is<Number>(elem_eval)) {
        throw RuntimeError("abs operator works with numbers only");
    }
    int64_t val = As<Number>(elem_eval)->GetValue();
    return Hp().Make<Number>(val < 0 ? -val : val);
}

Object* CheckListFunctor::Calc(const std::vector<Object*>& list, Object* scope) const {
    if (list.size() != 1) {
        throw RuntimeError("check-type operators works with 1-element list only");
    }
    Object* cur = list.back()->Eval(scope);
    if (!cur) {
        return Hp().Make<Boolean>(true);
    }
    while (true) {
        if (!Is<Cell>(cur)) {
            return Hp().Make<Boolean>(false);
        }
        Object* next = As<Cell>(cur)->GetSecond();
        if (!next) {
            return Hp().Make<Boolean>(true);
        }
        cur = next;
    }
    throw RuntimeError("check-list functor: unexpected behavior");
}
Object* CheckNullFunctor::Calc(const std::vector<Object*>& list, Object* scope) const {
    if (list.size() != 1) {
        throw RuntimeError("check-type operator works with 1-element list only");
    }
    return Hp().Make<Boolean>(!list.back()->Eval(scope));
}
Object* ConsFunctor::Calc(const std::vector<Object*>& list, Object* scope) const {
    if (list.size() != 2) {
        throw RuntimeError("cons function works with 1-element list only");
    }
    Object* first_eval = list[0]->Eval(scope);
    Object* second_eval = list.back()->Eval(scope);
    return Hp().Make<Cell>(first_eval, second_eval);
}

Object* ListFunctor::Calc(const std::vector<Object*>& list, Object*) const {
    return ListToObject(list);
}

Object* CarFunctor::Calc(const std::vector<Object*>& list, Object* scope) const {
    if (list.size() != 1) {
        throw RuntimeError("car function works with 1-element list only");
    }
    Object* first_eval = list.back()->Eval(scope);
    if (!Is<Cell>(first_eval)) {
        throw RuntimeError("car function works with cells only");
    }
    return As<Cell>(first_eval)->GetFirst();
}

Object* CdrFunctor::Calc(const std::vector<Object*>& list, Object* scope) const {
    if (list.size() != 1) {
        throw RuntimeError("cdr function works with 1-element list only");
    }
    Object* first_eval = list.back()->Eval(scope);
    if (!Is<Cell>(first_eval)) {
        throw RuntimeError("cdr function argument must be a cell");
    }
    return As<Cell>(first_eval)->GetSecond();
}

std::pair<std::vector<Object*>, int64_t> ListAbstractFunctor::Parse(
    const std::vector<Object*>& list, Object* scope) const {
    if (list.size() != 2) {
        throw RuntimeError("list- function works with 2-element list only");
    }
    Object* first_eval = list[0]->Eval(scope);
    Object* second_eval = list.back()->Eval(scope);
    if (!Is<Cell>(first_eval)) {
        throw RuntimeError("list- function first argument must be a cell");
    }
    if (!Is<Number>(second_eval)) {
        throw RuntimeError("list- function second argument must be a number");
    }
    int64_t val = As<Number>(second_eval)->GetValue();
    if (val < 0) {
        throw RuntimeError("list- function second argument must be non-negative");
    }
    std::vector<Object*> list_eval = ObjectToList(first_eval);
    return std::make_pair(list_eval, val);
}

Object* ListRefFunctor::Calc(const std::vector<Object*>& list, Object* scope) const {
    auto [list_eval, index] = Parse(list, scope);
    if (list_eval.size() <= index) {
        throw RuntimeError("list-ref function second argument must be less than the list size");
    }
    return list_eval[index];
}

Object* ListTailFunctor::Calc(const std::vector<Object*>& list, Object* scope) const {
    auto [list_eval, len] = Parse(list, scope);
    if (list_eval.size() < len) {
        throw RuntimeError(
            "list-tail function second argument must be less or equal than the list size");
    }
    std::vector<Object*> cut_list;
    for (size_t i = len; i < list_eval.size(); ++i) {
        cut_list.push_back(list_eval[i]);
    }
    return ListToObject(cut_list);
}

Object* IfOperator::Calc(const std::vector<Object*>& list, Object* scope) const {
    if (list.size() != 2 && list.size() != 3) {
        throw SyntaxError("operator if needs two or three arguments");
    }
    Object* condition_val = list[0]->Eval(scope);
    if (!Is<Boolean>(condition_val)) {
        throw RuntimeError("condition argument must be boolean");
    }
    if (As<Boolean>(condition_val)->GetValue()) {
        return list[1]->Eval(scope);
    } else if (!As<Boolean>(condition_val)->GetValue() && list.size() == 3) {
        return list[2]->Eval(scope);
    } else {
        return nullptr;
    }
}

Object* DefineOperator::Calc(const std::vector<Object*>& list, Object* scope) const {
    if (list.empty()) {
        throw SyntaxError("operator define needs at least one argument");
    }
    if (Is<Cell>(list[0])) {
        std::vector<Object*> instructions;
        for (size_t i = 1; i < list.size(); ++i) {
            instructions.push_back(list[i]);
        }
        return DefineFunction(list[0], instructions, scope);
    } else if (Is<Symbol>(list[0])) {
        if (list.size() != 2) {
            throw SyntaxError("operator define variable needs exactly 2 arguments");
        }
        return DefineVariable(list[0], list[1], scope);
    } else {
        throw SyntaxError("operator define works only with cells and symbols");
    }
}

Object* DefineOperator::DefineVariable(Object* var, Object* val, Object* scope) const {
    if (!Is<Symbol>(var)) {
        throw SyntaxError("operator define variable needs symbol as first argument");
    }
    if (!Is<Scope>(scope)) {
        throw std::logic_error("operator define variable needs scope as third argument");
    }
    As<Scope>(scope)->Define(As<Symbol>(var)->GetName(), val->Eval(scope));
    return As<Symbol>(var)->Eval(scope);
}

Object* DefineOperator::DefineFunction(Object* cell, std::vector<Object*> instructions,
                                       Object* scope) const {
    if (!Is<Scope>(scope)) {
        throw std::logic_error("operator define function: scope is not scope object");
    }
    if (!Is<Cell>(cell)) {
        throw std::logic_error("operator define function first argument must be cell");
    }
    std::vector<Object*> variables = ObjectToList(cell);
    if (variables.empty()) {
        throw SyntaxError("operator define function first argument is empty");
    }
    for (Object* var : variables) {
        if (!Is<Symbol>(var)) {
            throw SyntaxError("operator define function first argument contains non-symbol value");
        }
    }
    std::string function_name = As<Symbol>(variables[0])->GetName();
    std::vector<std::string> arg_names;
    arg_names.reserve(variables.size() - 1);
    for (size_t i = 1; i < variables.size(); ++i) {
        arg_names.push_back(As<Symbol>(variables[i])->GetName());
    }
    Lambda* lambda = Hp().Make<Lambda>(arg_names, instructions, As<Scope>(scope));
    As<Scope>(scope)->Define(function_name, lambda);
    return nullptr;
}

Object* SetOperator::Calc(const std::vector<Object*>& list, Object* scope) const {
    if (list.size() != 2) {
        throw SyntaxError("operator set needs exactly two arguments");
    }
    if (!Is<Scope>(scope)) {
        throw std::logic_error("operator set: scope needs to be scope");
    }
    if (!Is<Symbol>(list[0])) {
        throw RuntimeError("operator set first argument must be a symbol");
    }
    As<Scope>(scope)->Set(As<Symbol>(list[0])->GetName(), list[1]->Eval(scope));
    return list[0]->Eval(scope);
}

Object* LambdaMaker::Calc(const std::vector<Object*>& list, Object* scope) const {
    if (list.size() <= 1) {
        throw SyntaxError("lambda needs at least two arguments");
    }
    if (!Is<Cell>(list[0]) && list[0]) {
        throw SyntaxError("the first argument of lambda must be a cell");
    }
    std::vector<Object*> lambda_args = ObjectToList(list[0]);
    std::vector<std::string> lambda_arg_names;
    lambda_arg_names.reserve(lambda_args.size());
    for (Object* cur_arg : lambda_args) {
        if (!Is<Symbol>(cur_arg)) {
            throw SyntaxError("lambda args must be variables");
        }
        lambda_arg_names.push_back(As<Symbol>(cur_arg)->GetName());
    }
    std::vector<Object*> lambda_instructions;
    for (size_t i = 1; i < list.size(); ++i) {
        lambda_instructions.push_back(list[i]);
    }
    if (!Is<Scope>(scope)) {
        throw std::logic_error("lambda maker: scope must be a scope object");
    }
    return Hp().Make<Lambda>(lambda_arg_names, lambda_instructions, As<Scope>(scope));
}

Lambda::Lambda(const std::vector<std::string>& arg_names, std::vector<Object*> body,
               Scope* parent_scope)
    : arg_names_(arg_names), body_(body), parent_scope_(parent_scope) {
    for (Object* cur_instruction : body_) {
        AddDep(cur_instruction);
    }
    AddDep(parent_scope_);
}

Object* Lambda::Calc(const std::vector<Object*>& list, Object* outer_scope) const {
    if (body_.empty()) {
        throw std::logic_error("lambda body is empty at the moment of calculation");
    }
    if (list.size() < arg_names_.size()) {
        throw RuntimeError("too few arguments for lambda calculation");
    }
    if (arg_names_.size() < list.size()) {
        throw RuntimeError("too much arguments for lambda calculation");
    }
    Scope* current_call_scope = Hp().Make<Scope>(parent_scope_);
    for (size_t i = 0; i < arg_names_.size(); ++i) {
        current_call_scope->Define(arg_names_[i], list[i]->Eval(outer_scope));
    }
    for (size_t i = 0; i + 1 < body_.size(); ++i) {
        body_[i]->Eval(current_call_scope);
    }
    return body_.back()->Eval(current_call_scope);
}

Object* SetCarOperator::Calc(const std::vector<Object*>& list, Object* scope) const {
    if (list.size() != 2) {
        throw SyntaxError("set-car operator needs exactly 2 arguments");
    }
    if (!Is<Cell>(list[0]->Eval(scope))) {
        throw RuntimeError("set-car first argument must be cell");
    }
    Object* x = list[0]->Eval(scope);
    As<Cell>(x)->SetFirst(list[1]->Eval(scope));
    return nullptr;
}

Object* SetCdrOperator::Calc(const std::vector<Object*>& list, Object* scope) const {
    if (list.size() != 2) {
        throw SyntaxError("set-car operator needs exactly 2 arguments");
    }
    if (!Is<Cell>(list[0]->Eval(scope))) {
        throw RuntimeError("set-car first argument must be cell");
    }
    Object* x = list[0]->Eval(scope);
    As<Cell>(x)->SetSecond(list[1]->Eval(scope));
    return nullptr;
}
