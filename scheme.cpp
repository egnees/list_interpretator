#include "scheme.h"

#include <sstream>

std::string Interpreter::Run(const std::string& s) {
    std::stringstream ss{s};
    Tokenizer tokenizer{&ss};
    Object* node = Read(&tokenizer);

    if (!tokenizer.IsEnd()) {
        throw SyntaxError("too much tokens in the line");
    }

    if (!node) {
        throw RuntimeError("can not evaluate empty list");
    }

    Object* eval = node->Eval(base_scope_);
    std::string res = (eval ? eval->Serialize() : "()");
    ClearMemory();
    return res;
}

void Interpreter::ClearMemory() {
    Hp().CleanUp(base_scope_);
}

Interpreter::Interpreter() {
    Init();
}

Interpreter::~Interpreter() {
    for (auto [name, ptr] : functions_) {
        delete ptr;
    }
    delete base_scope_;
}

void Interpreter::Init() {
    functions_ = {{"quote", new QuoteFunctor()},

                  {"and", new BooleanFunctor([](bool a, bool b) { return a && b; }, false)},

                  {"or", new BooleanFunctor([](bool a, bool b) { return a || b; }, true)},

                  {"not", new BooleanNot()},

                  {"+", new NumberFunctor([](int64_t a, int64_t b) { return a + b; }, 0)},

                  {"*", new NumberFunctor([](int64_t a, int64_t b) { return a * b; }, 1)},

                  {"-", new NumberFunctor([](int64_t a, int64_t b) { return a - b; })},

                  {"/", new NumberFunctor([](int64_t a, int64_t b) { return a / b; })},

                  {"max", new NumberFunctor([](int64_t a, int64_t b) { return std::max(a, b); })},

                  {"min", new NumberFunctor([](int64_t a, int64_t b) { return std::min(a, b); })},

                  {"abs", new NumberAbs()},

                  {"<", new CompareFunctor([](int64_t a, int64_t b) { return a < b; })},

                  {">", new CompareFunctor([](int64_t a, int64_t b) { return a > b; })},

                  {"=", new CompareFunctor([](int64_t a, int64_t b) { return a == b; })},

                  {"<=", new CompareFunctor([](int64_t a, int64_t b) { return a <= b; })},

                  {">=", new CompareFunctor([](int64_t a, int64_t b) { return a >= b; })},

                  {"boolean?", new CheckTypeFunctor<Boolean>()},

                  {"number?", new CheckTypeFunctor<Number>()},

                  {"pair?", new CheckTypeFunctor<Cell>()},

                  {"symbol?", new CheckTypeFunctor<Symbol>()},

                  {"list?", new CheckListFunctor()},

                  {"null?", new CheckNullFunctor()},

                  {"car", new CarFunctor()},

                  {"cdr", new CdrFunctor()},

                  {"cons", new ConsFunctor()},

                  {"list", new ListFunctor()},

                  {"list-ref", new ListRefFunctor()},

                  {"list-tail", new ListTailFunctor()},

                  {"if", new IfOperator()},

                  {"define", new DefineOperator()},

                  {"set!", new SetOperator()},

                  {"lambda", new LambdaMaker()},

                  {"set-car!", new SetCarOperator()},

                  {"set-cdr!", new SetCdrOperator()}};

    base_scope_ = new Scope(functions_, nullptr);
}