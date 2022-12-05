#include "object.h"

/// Primitive class

class FunctionalObject : public Object {
public:
    virtual Object* Calc(Object*, Object*) const;
    virtual Object* Calc(const std::vector<Object*>&, Object*) const = 0;
    Object* Eval(Object*) const override {
        throw std::logic_error("can not eval functional object");
    }
    std::string Serialize() const override {
        return std::string("PrimitiveProcedure");
    }
    Object* AllocateCopy() const override {
        return nullptr;
    }
};

/// QuoteFunctor

class QuoteFunctor : public FunctionalObject {
public:
    Object* Calc(const std::vector<Object*>&, Object*) const override;
};

/// Booleans

class BooleanFunctor : public FunctionalObject {
public:
    BooleanFunctor(const std::function<bool(bool, bool)>& functor, bool stop_value)
        : functor_(functor), stop_value_(stop_value){};

    Object* Calc(const std::vector<Object*>&, Object*) const override;

private:
    std::function<bool(bool, bool)> functor_;
    bool stop_value_;
};

class BooleanNot : public FunctionalObject {
public:
    Object* Calc(const std::vector<Object*>&, Object*) const override;
};

/// NumberFunctors

class NumberFunctor : public FunctionalObject {
public:
    NumberFunctor(const std::function<int64_t(int64_t, int64_t)>& functor) : functor_(functor){};
    NumberFunctor(const std::function<int64_t(int64_t, int64_t)>& functor, int64_t first)
        : functor_(functor), first_(first){};

    Object* Calc(const std::vector<Object*>&, Object*) const override;

private:
    std::function<int64_t(int64_t, int64_t)> functor_;
    std::optional<int64_t> first_;
};

class NumberAbs : public FunctionalObject {
public:
    Object* Calc(const std::vector<Object*>&, Object*) const override;
};

/// Compare functors

class CompareFunctor : public FunctionalObject {
public:
    CompareFunctor(const std::function<bool(int64_t, int64_t)>& functor) : functor_(functor){};

    Object* Calc(const std::vector<Object*>&, Object*) const override;

private:
    std::function<bool(int64_t, int64_t)> functor_;
};

/// Check-type functors

template <class T>
class CheckTypeFunctor : public FunctionalObject {
public:
    Object* Calc(const std::vector<Object*>& list, Object* scope) const {
        if (list.size() != 1) {
            throw RuntimeError("check-type operators works with 1-element list only");
        }
        return Hp().Make<Boolean>(Is<T>(list.back()->Eval(scope)));
    }
};

class CheckListFunctor : public FunctionalObject {
public:
    Object* Calc(const std::vector<Object*>&, Object*) const override;
};

class CheckNullFunctor : public FunctionalObject {
public:
    Object* Calc(const std::vector<Object*>&, Object*) const override;
};

/// car, cdr and cons functors

class CarFunctor : public FunctionalObject {
public:
    Object* Calc(const std::vector<Object*>&, Object*) const override;
};

class CdrFunctor : public FunctionalObject {
public:
    Object* Calc(const std::vector<Object*>&, Object*) const override;
};

class ConsFunctor : public FunctionalObject {
public:
    Object* Calc(const std::vector<Object*>&, Object*) const override;
};

/// List functors

class ListFunctor : public FunctionalObject {
public:
    Object* Calc(const std::vector<Object*>&, Object*) const override;
};

class ListAbstractFunctor : public FunctionalObject {
protected:
    std::pair<std::vector<Object*>, int64_t> Parse(const std::vector<Object*>&, Object*) const;
};

class ListTailFunctor : public ListAbstractFunctor {
public:
    Object* Calc(const std::vector<Object*>&, Object*) const override;
};

class ListRefFunctor : public ListAbstractFunctor {
public:
    Object* Calc(const std::vector<Object*>&, Object*) const override;
};

/// If operator

class IfOperator : public FunctionalObject {
public:
    Object* Calc(const std::vector<Object*>&, Object*) const override;
};

/// Set and define operators

class DefineOperator : public FunctionalObject {
public:
    Object* Calc(const std::vector<Object*>&, Object*) const override;
    Object* DefineFunction(Object*, std::vector<Object*>, Object*) const;
    Object* DefineVariable(Object*, Object*, Object*) const;
};

class SetOperator : public FunctionalObject {
    Object* Calc(const std::vector<Object*>&, Object*) const override;
};

class SetCarOperator : public FunctionalObject {
    Object* Calc(const std::vector<Object*>&, Object*) const override;
};

class SetCdrOperator : public FunctionalObject {
    Object* Calc(const std::vector<Object*>&, Object*) const override;
};

/// Lambda operators

class LambdaMaker : public FunctionalObject {
    Object* Calc(const std::vector<Object*>&, Object*) const override;
};

class Lambda : public FunctionalObject {
public:
    Object* Calc(const std::vector<Object*>&, Object*) const override;
    Lambda(const std::vector<std::string>& arg_names, std::vector<Object*> body,
           Scope* parent_scope);

private:
    std::vector<std::string> arg_names_;
    std::vector<Object*> body_;
    Scope* parent_scope_;
};
