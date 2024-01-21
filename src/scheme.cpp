#include "scheme.h"
#include <cassert>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "error.h"
#include "functions.h"
#include "heap.h"
#include "object.h"
#include "tokenizer.h"

static Object* ReadSymbol(const std::string& name, Scope* current_scope) {
    if (current_scope == nullptr) {
        throw NameError("Unknow symbol '" + name + "'");
    }
    const auto& values = current_scope->GetValues();
    if (auto it = values.find(name); it != values.end()) {
        return it->second;
    }
    return ReadSymbol(name, current_scope->GetPrevios());
}
static Scope* FindScope(const std::string& name, Scope* current_scope) {
    if (current_scope == nullptr) {
        throw NameError("Unknow symbol '" + name + "'");
    }
    const auto& values = current_scope->GetValues();
    if (auto it = values.find(name); it != values.end()) {
        return current_scope;
    }
    return FindScope(name, current_scope->GetPrevios());
}

Interpreter::Interpreter() {
    static auto heap = GetHeap();
    default_scope_ = heap->Make<Scope>();
    current_scope_ = default_scope_;
    heap->AddRootDependency(default_scope_);

    Scope::free_index = 0;
    LambdaFunction::free_index = 0;

    // true/false symbols
    DefineValue("#t", heap->Make<Symbol>("#t"));
    DefineValue("#f", heap->Make<Symbol>("#f"));

    InitFunction(heap->Make<Quote>());

    // Integer funtions:
    InitFunction(heap->Make<IsNumber>());

    InitFunction(heap->Make<IsMonotonic>([](Int a, Int b) { return a == b; }, "="));
    InitFunction(heap->Make<IsMonotonic>([](Int a, Int b) { return a < b; }, "<"));
    InitFunction(heap->Make<IsMonotonic>([](Int a, Int b) { return a > b; }, ">"));
    InitFunction(heap->Make<IsMonotonic>([](Int a, Int b) { return a <= b; }, "<="));
    InitFunction(heap->Make<IsMonotonic>([](Int a, Int b) { return a >= b; }, ">="));

    InitFunction(heap->Make<IntOperations>([](Int a, Int b) { return a + b; }, 0, 0, -1, "+"));
    InitFunction(heap->Make<IntOperations>([](Int a, Int b) { return a - b; }, 0, 1, -1, "-"));
    InitFunction(heap->Make<IntOperations>([](Int a, Int b) { return a * b; }, 1, 0, -1, "*"));
    InitFunction(heap->Make<IntOperations>(
        [](Int a, Int b) {
            if (b == 0) {
                throw RuntimeError("division by zero");
            }
            return a / b;
        },
        1, 1, -1, "/"));

    InitFunction(
        heap->Make<IntOperations>([](Int a, Int b) { return std::max(a, b); }, 0, 1, -1, "max"));
    InitFunction(
        heap->Make<IntOperations>([](Int a, Int b) { return std::min(a, b); }, 0, 1, -1, "min"));

    InitFunction(heap->Make<IntSoloArgumentOperation>([](Int a) { return std::abs(a); }, "abs"));

    // boolean functions:
    InitFunction(heap->Make<IsBoolean>());
    InitFunction(heap->Make<Not>());
    InitFunction(heap->Make<And>());
    InitFunction(heap->Make<Or>());

    // list functions:
    InitFunction(heap->Make<IsPair>());
    InitFunction(heap->Make<IsNull>());
    InitFunction(heap->Make<IsList>());

    InitFunction(heap->Make<Cons>());
    InitFunction(heap->Make<Car>());
    InitFunction(heap->Make<Cdr>());

    InitFunction(heap->Make<List>());
    InitFunction(heap->Make<ListRef>());
    InitFunction(heap->Make<ListTail>());

    // advanced:
    InitFunction(heap->Make<IsSymbol>());
    InitFunction(heap->Make<If>());
    InitFunction(heap->Make<Define>());
    InitFunction(heap->Make<Set>());
    InitFunction(heap->Make<SetCar>());
    InitFunction(heap->Make<SetCdr>());
    InitFunction(heap->Make<LambdaFunction>());

    heap->DeleteUnuse();
}

std::string Interpreter::Run(const std::string& s) {
    static auto heap = GetHeap();
    heap->DeleteUnuse();
    std::istringstream input(s);
    Tokenizer tokenizer(&input);
    std::string answer;
    auto compiled = Compile(&tokenizer);
    if (!tokenizer.IsEnd()) {
        throw SyntaxError("expected end of line");
    }
    current_scope_ = default_scope_;
    auto executed = Execute(compiled);
    answer += Convert(executed);

    heap->DeleteUnuse();
    return answer;
}

Object* Interpreter::Compile(Tokenizer* tokenizer) {
    auto ans = Read(tokenizer);
    return ans;
}

Object* Interpreter::Execute(Object* to_execute) {
    if (to_execute == nullptr) {
        throw RuntimeError("can't execute nullptr");
    }
    if (Is<Number>(to_execute)) {
        return to_execute;
    }
    if (Is<Symbol>(to_execute)) {
        return CallSymbol(As<Symbol>(to_execute));
    }
    if (Is<BasicFunction>(to_execute)) {
        return to_execute;
    }
    if (Is<Lambda>(to_execute)) {
        return to_execute;
    }
    if (!Is<Cell>(to_execute)) {
        throw RuntimeError("unexpected object passed to function Interpreter::Execute");
    }

    auto call = As<Cell>(to_execute);
    call->SetFirst(Execute(call->GetFirst()));
    if (Is<BasicFunction>(call->GetFirst())) {
        return CallFunction(As<BasicFunction>(call->GetFirst()), call->GetSecond());
    }
    if (Is<Lambda>(call->GetFirst())) {
        return CallLambda(As<Lambda>(call->GetFirst()), call->GetSecond());
    }
    std::string trying_to_call = Convert(call->GetFirst());
    throw RuntimeError("can't call non function / lambda object '" + trying_to_call + "'");
}

std::string Interpreter::Convert(Object* to_convert) {
    if (to_convert == nullptr) {
        return "()";
    }
    return to_convert->ToString();
}

void Interpreter::DefineValue(const std::string& name, Object* object) {
    if (Is<BasicFunction>(object)) {
        current_scope_->AddValue(name, object);
    } else {
        current_scope_->AddValue(name, object->Copy());
    }
}

void Interpreter::SetValue(const std::string& name, Object* object) {
    auto scope = FindScope(name, current_scope_);
    if (Is<BasicFunction>(object)) {
        scope->AddValue(name, object);
    } else {
        scope->AddValue(name, object->Copy());
    }
}
Scope* Interpreter::GetCurrentScope() const {
    return current_scope_;
}

Object* Interpreter::CallSymbol(Symbol* symbol) {
    return ReadSymbol(symbol->GetName(), current_scope_);
}

Object* Interpreter::CallLambda(Lambda* lambda, Object* arguments) {
    static auto heap = GetHeap();
    auto [status, v] = ToVector(arguments);
    if (lambda->arguments.size() != v.size()) {
        std::string message =
            "invalid amount of arguments for lambda function '" + lambda->name + "'. ";
        message += "Expected " + std::to_string(lambda->arguments.size()) + ", ";
        message += "but got " + std::to_string(v.size());

        message += "\nargs:";
        for (size_t i = 0; i < v.size(); ++i) {
            message += " " + Execute(v[i])->ToString();
        }
        throw RuntimeError(message);
    }

    auto cs = current_scope_;
    auto ns = lambda->GetScope();

    std::string scope_name = "scope_" + std::to_string(Scope::free_index++);

    ns->AddScope(scope_name, heap->Make<Scope>(ns));
    ns = ns->GetScopes().at(scope_name);

    for (size_t i = 0; i < v.size(); ++i) {
        v[i] = Execute(v[i]);
    }
    for (size_t i = 0; i < v.size(); ++i) {
        ns->AddValue(lambda->arguments[i], v[i]);
    }

    current_scope_ = ns;

    auto [_, call_list] = ToVector(lambda->GetCall()->Copy());
    Object* answer = nullptr;
    for (auto& e : call_list) {
        answer = Execute(e);
    }
    lambda->GetScope()->RemoveScope(scope_name);
    current_scope_ = cs;
    return answer;
}

Object* Interpreter::CallFunction(BasicFunction* function, Object* arguments) {
    return function->Call(this, arguments);
}

void Interpreter::InitFunction(Object* function) {
    if (!Is<Function>(function)) {
        throw std::runtime_error("try to initializite function, that not derived from Function");
    }
    auto name = As<Function>(function)->GetName();
    default_scope_->AddValue(name, function);
}
