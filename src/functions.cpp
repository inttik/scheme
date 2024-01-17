#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>
#include <tuple>

#include "error.h"
#include "functions.h"
#include "int_type.h"
#include "object.h"
#include "scheme.h"
#include "tokenizer.h"

static auto heap = GetHeap();

std::pair<Status, std::vector<Object*>> ToVector(Object* object) {
    Status status = ProperList;
    std::vector<Object*> answer;
    while (Is<Cell>(object)) {
        auto current = As<Cell>(object)->GetFirst();
        answer.push_back(current);
        object = As<Cell>(object)->GetSecond();
    }
    if (object != nullptr) {
        status = ImproperList;
        answer.push_back(object);
    }
    return {status, answer};
}

static Object* ToList(const std::vector<Object*>& argument, size_t from, bool proper_list = true) {
    if (from >= argument.size()) {
        return nullptr;
    }
    if (!proper_list && from + 1 == argument.size()) {
        return argument[from];
    }
    Object* first = argument[from];
    Object* second = ToList(argument, from + 1, proper_list);
    return heap->Make<Cell>(first, second);
}

Function::Function(FunctionInfo function_info) : function_info_(function_info) {
}
Object* Function::Call(Interpreter* interpreter, Object* argument) {
    auto args = GetArguments(interpreter, argument);
    interpreter_ = interpreter;
    current_scope_ = interpreter->GetCurrentScope();
    return Apply(args);
}
std::vector<Object*> Function::GetArguments(Interpreter* interpreter, Object* argument) {
    auto& f_name = function_info_.name;
    auto& min_arg = function_info_.min_arg_count;
    auto& max_arg = function_info_.max_arg_count;
    auto& allow_improper = function_info_.allow_improper;
    auto& execute_all = function_info_.execute_args;
    auto& checker = function_info_.checker.checker;
    auto& bad_message = function_info_.checker.bad_check_msg;
    auto& syntax_error = function_info_.throw_syntax_error;
    auto& raw_argument = function_info_.raw_argument;

    if (raw_argument) {
        return {argument};
    }

    std::string funtion_text = ".";
    if (!f_name.empty()) {
        funtion_text = " for function '" + f_name + "'.";
    }

    auto [status, v] = ToVector(argument);
    if (!allow_improper && status == ImproperList) {
        std::string message = "improper list can't be interpreted as arguments" + funtion_text;
        throw SyntaxError(message);
    }
    if (v.size() < min_arg) {
        std::string message = "not enough argyments" + funtion_text;
        message += " Exepected at least " + std::to_string(min_arg);
        message += ", but got " + std::to_string(v.size()) + ".";
        if (syntax_error) {
            throw SyntaxError(message);
        }
        throw RuntimeError(message);
    }
    if (v.size() > max_arg) {
        std::string message = "too many argyments" + funtion_text;
        message += " Exepected at most " + std::to_string(max_arg);
        message += ", but got " + std::to_string(v.size()) + ".";
        if (syntax_error) {
            throw SyntaxError(message);
        }
        throw RuntimeError(message);
    }
    if (execute_all) {
        for (auto& arg : v) {
            arg = interpreter->Execute(arg);
        }
    }
    if (checker != nullptr) {
        for (size_t i = 0; i < v.size(); ++i) {
            if (!checker(v[i])) {
                std::string message = "bad argument #" + std::to_string(i) + funtion_text;
                if (!bad_message.empty()) {
                    message += "\ninfo: " + bad_message;
                }
                throw RuntimeError(message);
            }
        }
    }
    return v;
}

std::string Function::GetName() const {
    return function_info_.name;
}
std::string Function::ToString() const {
    return "<function '" + function_info_.name + "'>";
}

// helpers
static bool IsNumCheck(Object* object) {
    return Is<Number>(object);
}

static Checker is_num_checker{
    .checker = IsNumCheck,
    .bad_check_msg = "accepts only numbers",
};

static bool IsTrue(Object* object) {
    if (!Is<Symbol>(object)) {
        return true;
    }
    if (As<Symbol>(object)->GetName() != "#f") {
        return true;
    }
    return false;
}

// realisations
IsNumber::IsNumber()
    : Function({
          .min_arg_count = 1,
          .max_arg_count = 1,
          .name = "number?",
      }) {
}
Object* IsNumber::Apply(const ArgsType& arguments) {
    if (Is<Number>(arguments[0])) {
        return heap->Make<Symbol>("#t");
    }
    return heap->Make<Symbol>("#f");
}

IsMonotonic::IsMonotonic(bool (*comparator)(Int, Int), std::string function_name)
    : Function{{
          .min_arg_count = 0,
          .max_arg_count = static_cast<size_t>(-1),
          .name = function_name,
          .checker = is_num_checker,
      }},
      comparator_(comparator) {
}
Object* IsMonotonic::Apply(const ArgsType& arguments) {
    if (arguments.size() < 2) {
        return heap->Make<Symbol>("#t");
    }
    auto prev = As<Number>(arguments[0])->GetValue();
    for (size_t i = 1; i < arguments.size(); ++i) {
        auto current = As<Number>(arguments[i])->GetValue();
        if (!comparator_(prev, current)) {
            return heap->Make<Symbol>("#f");
        }
        prev = current;
    }
    return heap->Make<Symbol>("#t");
}

IntOperations::IntOperations(Int (*apply)(Int, Int), Int default_value, size_t min_arg_count,
                             size_t max_arg_count, std::string function_name)
    : Function({
          .min_arg_count = min_arg_count,
          .max_arg_count = max_arg_count,
          .name = function_name,
          .checker = is_num_checker,
      }),
      default_value_(default_value),
      apply_(apply) {
}
Object* IntOperations::Apply(const ArgsType& arguments) {
    if (arguments.empty()) {
        return heap->Make<Number>(default_value_);
    }
    auto answer = As<Number>(arguments[0])->GetValue();
    for (size_t i = 1; i < arguments.size(); ++i) {
        auto current = As<Number>(arguments[i])->GetValue();
        answer = apply_(answer, current);
    }
    return heap->Make<Number>(answer);
}

IntSoloArgumentOperation::IntSoloArgumentOperation(Int (*apply)(Int), std::string function_name)
    : Function({
          .min_arg_count = 1,
          .max_arg_count = 1,
          .name = function_name,
          .checker = is_num_checker,
      }),
      apply_(apply) {
}
Object* IntSoloArgumentOperation::Apply(const ArgsType& arguments) {
    Int answer = apply_(As<Number>(arguments[0])->GetValue());
    return heap->Make<Number>(answer);
}

Quote::Quote()
    : Function({
          .min_arg_count = 1,
          .max_arg_count = 1,
          .name = "quote",
          .execute_args = false,
      }) {
}
Object* Quote::Apply(const ArgsType& arguments) {
    return arguments[0];
}

IsBoolean::IsBoolean()
    : Function({
          .min_arg_count = 1,
          .max_arg_count = 1,
          .name = "boolean?",
      }) {
}
Object* IsBoolean::Apply(const ArgsType& arguments) {
    if (!Is<Symbol>(arguments[0])) {
        return heap->Make<Symbol>("#f");
    }
    auto str = As<Symbol>(arguments[0])->GetName();
    if (str == "#t" || str == "#f") {
        return heap->Make<Symbol>("#t");
    }
    return heap->Make<Symbol>("#f");
}

Not::Not()
    : Function({
          .min_arg_count = 1,
          .max_arg_count = 1,
          .name = "not",
      }) {
}
Object* Not::Apply(const ArgsType& arguments) {
    if (IsTrue(arguments[0])) {
        return heap->Make<Symbol>("#f");
    }
    return heap->Make<Symbol>("#t");
}

And::And()
    : Function({
          .min_arg_count = 0,
          .max_arg_count = static_cast<size_t>(-1),
          .name = "and",
          .execute_args = false,
      }) {
}
Object* And::Apply(const ArgsType& arguments) {
    Object* last_expr = heap->Make<Symbol>("#t");
    for (auto& arg : arguments) {
        last_expr = interpreter_->Execute(arg);
        if (!IsTrue(last_expr)) {
            break;
        }
    }
    return last_expr;
}

Or::Or()
    : Function({
          .min_arg_count = 0,
          .max_arg_count = static_cast<size_t>(-1),
          .name = "or",
          .execute_args = false,
      }) {
}
Object* Or::Apply(const ArgsType& arguments) {
    Object* last_expr = heap->Make<Symbol>("#f");
    for (auto& arg : arguments) {
        last_expr = interpreter_->Execute(arg);
        if (IsTrue(last_expr)) {
            break;
        }
    }
    return last_expr;
}

IsPair::IsPair()
    : Function({
          .min_arg_count = 1,
          .max_arg_count = 1,
          .name = "pair?",
      }) {
}
Object* IsPair::Apply(const ArgsType& arguments) {
    auto [status, v] = ToVector(arguments[0]);
    if (v.size() == 2) {
        return heap->Make<Symbol>("#t");
    }
    return heap->Make<Symbol>("#f");
}

IsNull::IsNull()
    : Function({
          .min_arg_count = 1,
          .max_arg_count = 1,
          .name = "null?",
      }) {
}
Object* IsNull::Apply(const ArgsType& arguments) {
    auto [status, v] = ToVector(arguments[0]);
    if (v.empty()) {
        return heap->Make<Symbol>("#t");
    }
    return heap->Make<Symbol>("#f");
}

IsList::IsList()
    : Function({
          .min_arg_count = 1,
          .max_arg_count = 1,
          .name = "list?",
      }) {
}
Object* IsList::Apply(const ArgsType& arguments) {
    auto [status, v] = ToVector(arguments[0]);
    if (status == ProperList) {
        return heap->Make<Symbol>("#t");
    }
    return heap->Make<Symbol>("#f");
}

Cons::Cons()
    : Function({
          .min_arg_count = 2,
          .max_arg_count = 2,
          .name = "cons",
      }) {
}
Object* Cons::Apply(const ArgsType& arguments) {
    return heap->Make<Cell>(arguments[0], arguments[1]);
}

Car::Car()
    : Function({
          .min_arg_count = 1,
          .max_arg_count = 1,
          .name = "car",
          .raw_argument = true,
      }) {
}
Object* Car::Apply(const ArgsType& arguments) {
    if (!Is<Cell>(arguments[0])) {
        throw RuntimeError("using car on empty list / not list");
    }
    auto data = interpreter_->Execute(As<Cell>(arguments[0])->GetFirst());
    if (!Is<Cell>(data)) {
        throw RuntimeError("using car on empty list / not list");
    }
    return As<Cell>(data)->GetFirst();
}

Cdr::Cdr()
    : Function({
          .min_arg_count = 1,
          .max_arg_count = 1,
          .name = "cdr",
          .raw_argument = true,
      }) {
}
Object* Cdr::Apply(const ArgsType& arguments) {
    if (!Is<Cell>(arguments[0])) {
        throw RuntimeError("using cdr on empty list / not list");
    }
    auto data = interpreter_->Execute(As<Cell>(arguments[0])->GetFirst());
    if (!Is<Cell>(data)) {
        throw RuntimeError("using cdr on empty list / not list");
    }
    return As<Cell>(data)->GetSecond();
}

List::List()
    : Function({
          .min_arg_count = 0,
          .max_arg_count = static_cast<size_t>(-1),
          .name = "list",
      }) {
}
Object* List::Apply(const ArgsType& arguments) {
    return ToList(arguments, 0);
}

ListRef::ListRef()
    : Function({
          .min_arg_count = 2,
          .max_arg_count = 2,
          .name = "list-ref",
      }) {
}
Object* ListRef::Apply(const ArgsType& arguments) {
    auto [status, v] = ToVector(arguments[0]);
    if (!Is<Number>(arguments[1])) {
        throw RuntimeError("argument #1 for function list-ref shoud be Number");
    }
    Int index = As<Number>(arguments[1])->GetValue();
    if (index < 0 || index >= static_cast<Int>(v.size())) {
        throw RuntimeError("argument #1 for function list-ref is out of range");
    }
    return v[index];
}

ListTail::ListTail()
    : Function({
          .min_arg_count = 2,
          .max_arg_count = 2,
          .name = "list-tail",
      }) {
}
Object* ListTail::Apply(const ArgsType& arguments) {
    if (!Is<Number>(arguments[1])) {
        throw RuntimeError("argument #1 for funtion list-tail shoud be Number");
    }
    auto ans = arguments[0];
    Int index = As<Number>(arguments[1])->GetValue();
    while (index > 0) {
        if (ans == nullptr || !Is<Cell>(ans)) {
            throw RuntimeError("argument #2 for function list-tail is out of range");
        }
        --index;
        ans = As<Cell>(ans)->GetSecond();
    }
    return ans;
}

IsSymbol::IsSymbol()
    : Function({
          .min_arg_count = 1,
          .max_arg_count = 1,
          .name = "symbol?",
      }) {
}
Object* IsSymbol::Apply(const ArgsType& arguments) {
    if (Is<Symbol>(arguments[0])) {
        return heap->Make<Symbol>("#t");
    }
    return heap->Make<Symbol>("#f");
}

If::If()
    : Function({
          .min_arg_count = 2,
          .max_arg_count = 3,
          .name = "if",
          .execute_args = false,
          .throw_syntax_error = true,
      }) {
}
Object* If::Apply(const ArgsType& arguments) {
    auto value = interpreter_->Execute(arguments[0]);
    if (IsTrue(value)) {
        return interpreter_->Execute(arguments[1]);
    } else {
        if (arguments.size() < 3) {
            return nullptr;
        }
        return interpreter_->Execute(arguments[2]);
    }
}

Lambda* CreateLambda(const std::vector<Object*>& args_names, int from, Scope* scope,
                     Object* calls) {
    auto answer = heap->Make<Lambda>();
    answer->name = "lambda_" + std::to_string(LambdaFunction::free_index++);

    answer->SetScope(scope);

    for (size_t i = from; i < args_names.size(); ++i) {
        auto& la = args_names[i];
        if (!Is<Symbol>(la)) {
            throw RuntimeError("only symbols could be lambda arguments.");
        }
        answer->arguments.push_back(As<Symbol>(la)->GetName());
    }
    answer->SetCall(calls);
    return answer;
}

Define::Define()
    : Function({
          .min_arg_count = 2,
          .max_arg_count = static_cast<size_t>(-1),
          .name = "define",
          .execute_args = false,
          .throw_syntax_error = true,
      }) {
}
Object* Define::Apply(const ArgsType& arguments) {
    if (Is<Symbol>(arguments[0])) {
        if (arguments.size() > 2) {
            throw SyntaxError("too many arguments for function 'define'");
        }
        auto value = interpreter_->Execute(arguments[1]);
        interpreter_->DefineValue(As<Symbol>(arguments[0])->GetName(), value);
        return heap->Make<Empty>();
    }
    // lambda sugar
    if (!Is<Cell>(arguments[0])) {
        throw SyntaxError("incorrect usage of define");
    }
    auto [_, lambda_params] = ToVector(arguments[0]);
    if (!Is<Symbol>(lambda_params[0])) {
        throw SyntaxError("incorrect function name");
    }
    auto calls = ToList(arguments, 1);
    auto ans = CreateLambda(lambda_params, 1, current_scope_, calls);
    interpreter_->DefineValue(As<Symbol>(lambda_params[0])->GetName(), ans);
    return heap->Make<Empty>();
}

Set::Set()
    : Function({
          .min_arg_count = 2,
          .max_arg_count = 2,
          .name = "set!",
          .execute_args = false,
          .throw_syntax_error = true,
      }) {
}
Object* Set::Apply(const ArgsType& arguments) {
    if (!Is<Symbol>(arguments[0])) {
        throw RuntimeError("argument #0 for function set! shoud be Symbol");
    }
    auto value = interpreter_->Execute(arguments[1]);
    interpreter_->SetValue(As<Symbol>(arguments[0])->GetName(), value);
    return heap->Make<Empty>();
}

SetCar::SetCar()
    : Function({
          .min_arg_count = 2,
          .max_arg_count = 2,
          .name = "set-car!",
      }) {
}
Object* SetCar::Apply(const ArgsType& arguments) {
    auto& var = arguments[0];
    if (!Is<Cell>(var)) {
        throw RuntimeError("argument #0 for funtion set-car! shoud be Symbol converting to pair");
    }
    As<Cell>(var)->SetFirst(arguments[1]);
    return heap->Make<Empty>();
}

SetCdr::SetCdr()
    : Function({
          .min_arg_count = 2,
          .max_arg_count = 2,
          .name = "set-cdr!",
      }) {
}
Object* SetCdr::Apply(const ArgsType& arguments) {
    auto& var = arguments[0];
    if (!Is<Cell>(var)) {
        throw RuntimeError("argument #0 for funtion set-car! shoud be Symbol converting to pair");
    }
    As<Cell>(var)->SetSecond(arguments[1]);
    return heap->Make<Empty>();
}

LambdaFunction::LambdaFunction()
    : Function({
          .min_arg_count = 2,
          .max_arg_count = static_cast<size_t>(-1),
          .name = "lambda",
          .execute_args = false,
          .throw_syntax_error = true,
      }) {
}
Object* LambdaFunction::Apply(const ArgsType& arguments) {
    auto [status, lambda_args] = ToVector(arguments[0]);
    auto ans = CreateLambda(lambda_args, 0, current_scope_, ToList(arguments, 1));
    return ans;
}
