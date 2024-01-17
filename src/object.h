#pragma once

#include <set>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <map>
#include <vector>

#include "int_type.h"

class Heap;

class Object {
    friend class Heap;

protected:
    Object();

public:
    Object& operator=(const Object& other) = delete;
    virtual Object* Copy() const = 0;
    virtual std::string ToString() const = 0;
    virtual ~Object() = default;

protected:
    void AddDependency(Object* other);
    void RemoveDependency(Object* other);

private:
    std::multiset<Object*> dependency_;
    bool mark_bit_;
};

class Empty : public Object {
    friend class Heap;

private:
    Empty() = default;

public:
    Object* Copy() const override;
    std::string ToString() const override;
};

class Number : public Object {
    friend class Heap;

private:
    Number(Int value);

public:
    Int GetValue() const;

    Object* Copy() const override;
    std::string ToString() const override;

private:
    Int value_;
};

class Symbol : public Object {
    friend class Heap;

private:
    Symbol(const std::string& symbol);

public:
    const std::string& GetName() const;

    Object* Copy() const override;
    std::string ToString() const override;

private:
    std::string symbol_;
};

class Cell : public Object {
    friend class Heap;

private:
    Cell(Object* first, Object* second);

public:
    Object* GetFirst() const;
    Object* GetSecond() const;

    void SetFirst(Object*);
    void SetSecond(Object*);

    Object* Copy() const override;
    std::string ToString() const override;

private:
    std::string ToString(bool in_list) const;

    Object* first_;
    Object* second_;
};

class Interpreter;

struct BasicFunction : Object {
    friend class Heap;

protected:
    BasicFunction() = default;

public:
    virtual Object* Call(Interpreter* interpreter, Object* argument) = 0;
    virtual ~BasicFunction() = default;

    Object* Copy() const override;
};

struct Scope : Object {
    friend class Heap;

private:
    Scope();
    Scope(Scope* previos);

public:
    inline static size_t free_index;

    Object* Copy() const override;
    std::string ToString() const override;

    void AddScope(const std::string& name, Scope* scope);
    void AddValue(const std::string& name, Object* value);

    void RemoveScope(const std::string& name);

    const std::map<std::string, Scope*>& GetScopes() const;
    const std::map<std::string, Object*>& GetValues() const;
    Scope* GetPrevios() const;

private:
    Scope* previos_;
    std::map<std::string, Scope*> scopes_;
    std::map<std::string, Object*> values_;
};

struct Lambda : Object {
    friend class Heap;

private:
    Lambda() = default;

public:
    std::vector<std::string> arguments;
    std::string name;

    Object* Copy() const override;
    std::string ToString() const override;

    Scope* GetScope() const;
    Object* GetCall() const;
    void SetScope(Scope* scope);
    void SetCall(Object* call);

private:
    Scope* my_scope_;
    Object* call_;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and convertion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
T* As(Object* obj) {
    auto ans = dynamic_cast<T*>(obj);
    if (ans == nullptr) {
        throw std::runtime_error("using As<T> while type is incorrect");
    }
    return ans;
}

template <class T>
bool Is(Object* obj) {
    auto ans = dynamic_cast<T*>(obj);
    if (ans == nullptr) {
        return false;
    }
    return true;
}
