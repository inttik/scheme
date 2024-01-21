#pragma once

#include <memory>
#include <vector>

#include "object.h"
#include "scheme.h"
#include "heap.h"

enum Status {
    ProperList,
    ImproperList,
};

std::pair<Status, std::vector<Object*>> ToVector(Object* object);

using ArgsType = std::vector<Object*>;

struct Checker {
    bool (*checker)(Object*) = nullptr;
    std::string bad_check_msg = std::string();
};

struct FunctionInfo {
    size_t min_arg_count;
    size_t max_arg_count;
    std::string name;
    bool allow_improper = false;
    bool execute_args = true;
    Checker checker{
        .checker = nullptr,
        .bad_check_msg = std::string(),
    };
    bool throw_syntax_error = false;
    bool raw_argument = false;
};

struct Function : public BasicFunction {
    explicit Function(FunctionInfo function_info);
    Object* Call(Interpreter* interpreter, Object* argument) override;
    virtual Object* Apply(const ArgsType& arguments) = 0;

    std::string GetName() const;
    std::string ToString() const override;

protected:
    Interpreter* interpreter_;
    Scope* current_scope_;

private:
    ArgsType GetArguments(Interpreter* interpreter, Object* argument);
    FunctionInfo function_info_;
};

struct IsNumber : Function {
    IsNumber();
    Object* Apply(const ArgsType& arguments) override;
};

struct IsMonotonic : public Function {
    IsMonotonic(bool (*comparator)(Int, Int), std::string function_name);
    Object* Apply(const ArgsType& arguments) override;

private:
    bool (*comparator_)(Int, Int);
};

struct IntOperations : public Function {
    IntOperations(Int (*apply)(Int, Int), Int default_value, size_t min_arg_count,
                  size_t max_arg_count, std::string function_name);
    Object* Apply(const ArgsType& arguments) override;

private:
    Int default_value_;
    Int (*apply_)(Int, Int);
};

struct IntSoloArgumentOperation : public Function {
    IntSoloArgumentOperation(Int (*apply)(Int), std::string function_name);
    Object* Apply(const ArgsType& arguments) override;

private:
    Int (*apply_)(Int);
};

struct Quote : public Function {
    Quote();
    Object* Apply(const ArgsType& arguments) override;
};

struct IsBoolean : public Function {
    IsBoolean();
    Object* Apply(const ArgsType& arguments) override;
};

struct Not : public Function {
    Not();
    Object* Apply(const ArgsType& arguments) override;
};

struct And : public Function {
    And();
    Object* Apply(const ArgsType& arguments) override;
};

struct Or : public Function {
    Or();
    Object* Apply(const ArgsType& arguments) override;
};

struct IsPair : public Function {
    IsPair();
    Object* Apply(const ArgsType& arguments) override;
};

struct IsNull : public Function {
    IsNull();
    Object* Apply(const ArgsType& arguments) override;
};

struct IsList : public Function {
    IsList();
    Object* Apply(const ArgsType& arguments) override;
};

struct Cons : public Function {
    Cons();
    Object* Apply(const ArgsType& arguments) override;
};

struct Car : public Function {
    Car();
    Object* Apply(const ArgsType& arguments) override;
};

struct Cdr : public Function {
    Cdr();
    Object* Apply(const ArgsType& arguments) override;
};

struct List : public Function {
    List();
    Object* Apply(const ArgsType& arguments) override;
};

struct ListRef : public Function {
    ListRef();
    Object* Apply(const ArgsType& arguments) override;
};

struct ListTail : public Function {
    ListTail();
    Object* Apply(const ArgsType& arguments) override;
};

struct IsSymbol : public Function {
    IsSymbol();
    Object* Apply(const ArgsType& arguments) override;
};

struct If : public Function {
    If();
    Object* Apply(const ArgsType& arguments) override;
};

struct Define : public Function {
    Define();
    Object* Apply(const ArgsType& arguments) override;
};

struct Set : public Function {
    Set();
    Object* Apply(const ArgsType& arguments) override;
};

struct SetCar : public Function {
    SetCar();
    Object* Apply(const ArgsType& arguments) override;
};

struct SetCdr : public Function {
    SetCdr();
    Object* Apply(const ArgsType& arguments) override;
};

struct LambdaFunction : public Function {
    inline static size_t free_index;
    LambdaFunction();
    Object* Apply(const ArgsType& arguments) override;
};
