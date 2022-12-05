#pragma once

#include "error.h"

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <utility>
#include <map>
#include <algorithm>
#include <set>

class Object : public std::enable_shared_from_this<Object> {
public:
    Object(const Object& other) = delete;
    Object() = default;
    virtual ~Object() = default;
    virtual Object* Eval(Object*) const = 0;
    virtual std::string Serialize() const = 0;
    virtual Object* AllocateCopy() const = 0;
    void Mark() {
        marked_ = true;
    }
    void Unmark() {
        marked_ = false;
    }
    bool Marked() const {
        return marked_;
    }
    void AddDep(Object* other) {
        dep_.push_back(other);
    }
    void RmDep(Object* other) {
        auto it = std::find(dep_.begin(), dep_.end(), other);
        if (it != dep_.end()) {
            dep_.erase(it);
        }
    }
    const std::vector<Object*>& Dep() const {
        return dep_;
    }

private:
    bool marked_;
    std::vector<Object*> dep_;
};

class Scope;

class Heap {
public:
    template <typename T, typename... Args>
    T* Make(Args&&... args) {
        T* x = new T(std::forward<Args>(args)...);
        objects_.push_back(dynamic_cast<Object*>(x));
        return x;
    }
    template <typename T>
    T* Clone(T* ptr) {
        if (!Is<Object>(ptr)) {
            throw std::logic_error("In clone method Object is not base of ptr");
        }
        T* copy = As<T>(As<Object>(ptr)->AllocateCopy());
        objects_.push_back(copy);
        return copy;
    }
    ~Heap() {
        for (Object* alive : objects_) {
            if (alive) {
                delete alive;
            }
        }
    }
    void CleanUp(Object* root);

private:
    void Mark(Object* root);

    std::vector<Object*> objects_;
    std::set<Object*> used_;
};

Heap& Hp();

class Number : public Object {
public:
    explicit Number(int64_t value) : value_{value} {};
    int64_t GetValue() const {
        return value_.x;
    }
    Object* Eval(Object*) const override {
        return Hp().Make<Number>(value_.x);
    }
    std::string Serialize() const override {
        return std::to_string(value_.x);
    }
    Object* AllocateCopy() const override {
        return new Number(value_.x);
    }

private:
    struct Mint64 {
        int64_t x;
    } value_;
};

class Boolean : public Object {
public:
    explicit Boolean(bool value) : value_{value} {};
    bool GetValue() const {
        return value_.x;
    }
    Object* Eval(Object*) const override {
        return Hp().Make<Boolean>(value_.x);
    }
    std::string Serialize() const override {
        return value_.x ? "#t" : "#f";
    }
    Object* AllocateCopy() const override {
        return new Boolean(value_.x);
    }

private:
    struct Mbool {
        bool x;
    } value_;
};

class Symbol : public Object {
public:
    Symbol(const std::string name) : name_(name) {
    }
    const std::string& GetName() const {
        return name_;
    }
    Object* Eval(Object* scope) const override;
    std::string Serialize() const override {
        return name_;
    }
    Object* AllocateCopy() const override {
        return new Symbol(name_);
    }

private:
    std::string name_;
};

class Cell : public Object {
public:
    Cell(Object* first, Object* second = nullptr) : first_(first), second_(second) {
        AddDep(first_);
        AddDep(second_);
    };
    Object* GetFirst() const {
        return first_;
    }
    Object* GetSecond() const {
        return second_;
    }
    void SetFirst(Object* ptr) {
        RmDep(first_);
        first_ = ptr;
        AddDep(first_);
    }
    void SetSecond(Object* ptr) {
        RmDep(second_);
        second_ = ptr;
        AddDep(second_);
    }
    Object* Eval(Object* scope) const override;
    std::string Serialize() const override;
    Object* AllocateCopy() const override {
        return new Cell(first_, second_);
    }

private:
    Object* first_;
    Object* second_;
};

class Scope : public Object {
public:
    Object* Eval(Object*) const override {
        throw std::logic_error("Can not eval scope object");
    }
    std::string Serialize() const override {
        throw std::logic_error("Can not serialize scope object");
    }
    Scope(const std::map<std::string, Object*>& variables = {})
        : variables_(variables), parent_(nullptr) {
        for (auto [str, var] : variables_) {
            AddDep(var);
        }
        AddDep(parent_);
    }
    Scope(const std::map<std::string, Object*>& variables, Scope* parent)
        : variables_(variables), parent_(parent) {
        for (auto [str, var] : variables_) {
            AddDep(var);
        }
        AddDep(parent_);
    }
    Scope(Scope* parent) : parent_(parent) {
        AddDep(parent_);
    }
    Object* Find(const std::string& name) {
        if (variables_.contains(name)) {
            return variables_[name];
        }
        return parent_ ? parent_->Find(name) : nullptr;
    }
    bool Exists(const std::string& name) const {
        if (variables_.contains(name)) {
            return true;
        }
        return parent_ && parent_->Exists(name);
    }
    void Set(const std::string& name, Object* value) {
        if (!Exists(name)) {
            throw NameError("cant recognize name " + name);
        }
        SetForce(name, value);
    }
    void Define(const std::string& name, Object* value) {
        variables_[name] = value;
        AddDep(value);
    }
    Object* AllocateCopy() const override {
        return new Scope(variables_, parent_);
    }

private:
    void SetForce(const std::string& name, Object* value) {
        if (variables_.contains(name)) {
            RmDep(variables_[name]);
            variables_[name] = value;
            AddDep(value);
        } else if (parent_) {
            parent_->SetForce(name, value);
        } else {
            throw std::logic_error(
                "scope: set force: current scope variables not contain name and parent is null");
        }
    }
    std::map<std::string, Object*> variables_;
    Scope* parent_;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and convertion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
T* As(Object* obj) noexcept {
    return dynamic_cast<T*>(obj);
}

template <class T>
bool Is(Object* obj) {
    return As<T>(obj) != nullptr;
}
