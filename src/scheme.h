#pragma once

#include <memory>
#include <map>
#include <string>

#include "object.h"
#include "parser.h"
#include "tokenizer.h"

class Interpreter {
public:
    Interpreter();
    std::string Run(const std::string&);
    Object* Compile(Tokenizer*);
    Object* Execute(Object*);
    static std::string Convert(Object*);

    void DefineValue(const std::string& name, Object* object);
    void SetValue(const std::string& name, Object* object);

    Scope* GetCurrentScope() const;

private:
    Object* CallSymbol(Symbol* symbol);
    Object* CallLambda(Lambda* lambda, Object* arguments);
    Object* CallFunction(BasicFunction* function, Object* arguments);

    void InitFunction(Object* function);

    Scope* default_scope_;
    Scope* current_scope_;
};
