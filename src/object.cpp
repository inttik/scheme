#include "object.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <typeinfo>

#include "heap.h"

Object::Object() : mark_bit_(false) {
}
void Object::AddDependency(Object* other) {
    if (other == nullptr) {
        return;
    }
    dependency_.insert(other);
}
void Object::RemoveDependency(Object* other) {
    if (other == nullptr) {
        return;
    }
    if (dependency_.find(other) == dependency_.end()) {
        return;
    }
    dependency_.erase(dependency_.find(other));
}

Object* Empty::Copy() const {
    static auto heap = GetHeap();
    return heap->Make<Empty>();
};
std::string Empty::ToString() const {
    return "";
}

Number::Number(Int value) {
    value_ = value;
}
Int Number::GetValue() const {
    return value_;
}
Object* Number::Copy() const {
    static auto heap = GetHeap();
    return heap->Make<Number>(value_);
}
std::string Number::ToString() const {
    return std::to_string(value_);
}

Symbol::Symbol(const std::string& symbol) {
    symbol_ = symbol;
}
const std::string& Symbol::GetName() const {
    return symbol_;
}
Object* Symbol::Copy() const {
    static auto heap = GetHeap();
    return heap->Make<Symbol>(symbol_);
}
std::string Symbol::ToString() const {
    return symbol_;
}

Cell::Cell(Object* first, Object* second) {
    AddDependency(first);
    AddDependency(second);
    first_ = first;
    second_ = second;
}
Object* Cell::GetFirst() const {
    return first_;
}
Object* Cell::GetSecond() const {
    return second_;
}
void Cell::SetFirst(Object* first) {
    RemoveDependency(first_);
    first_ = first;
    AddDependency(first_);
}
void Cell::SetSecond(Object* second) {
    RemoveDependency(second_);
    second_ = second;
    AddDependency(second_);
}
Object* Cell::Copy() const {
    static auto heap = GetHeap();
    Object* n_f = nullptr;
    Object* n_s = nullptr;
    if (first_ != nullptr) {
        n_f = first_->Copy();
    }
    if (second_ != nullptr) {
        n_s = second_->Copy();
    }
    return heap->Make<Cell>(n_f, n_s);
}
std::string Cell::ToString() const {
    return ToString(false);
}
std::string Cell::ToString(bool in_list) const {
    std::string answer;
    if (!in_list) {
        answer += "(";
    }
    if (first_ == nullptr) {
        answer += "()";
    } else if (Is<Cell>(first_)) {
        answer += As<Cell>(first_)->ToString(false);
    } else {
        answer += first_->ToString();
    }
    if (second_ == nullptr) {
        answer += ")";
        return answer;
    }
    if (Is<Cell>(second_)) {
        answer += " ";
        answer += As<Cell>(second_)->ToString(true);
        return answer;
    } else {
        answer += " . ";
        answer += second_->ToString();
        answer += ")";
        return answer;
    }
}

Object* BasicFunction::Copy() const {
    throw std::runtime_error("basic function is not copyable");
}

Scope::Scope() {
    previos_ = nullptr;
}
Scope::Scope(Scope* previos) {
    this->previos_ = previos;
}
void Scope::AddScope(const std::string& name, Scope* scope) {
    if (scopes_.find(name) != scopes_.end()) {
        RemoveDependency(scopes_[name]);
    }
    AddDependency(scope);
    scopes_[name] = scope;
}
void Scope::AddValue(const std::string& name, Object* value) {
    if (values_.find(name) != values_.end()) {
        RemoveDependency(values_[name]);
    }
    AddDependency(value);
    values_[name] = value;
}
void Scope::RemoveScope(const std::string& name) {
    if (scopes_.find(name) != scopes_.end()) {
        RemoveDependency(scopes_[name]);
        scopes_.erase(name);
    }
}
Object* Scope::Copy() const {
    throw std::runtime_error("scope is not copyable");
}
std::string Scope::ToString() const {
    std::string ans = "<Scope (" + std::to_string(reinterpret_cast<std::uintptr_t>(this)) + "):\n";
    ans += "previos = " + std::to_string(reinterpret_cast<std::uintptr_t>(previos_)) + ",\n";
    ans += "values:\n";
    for (auto [str, obj] : values_) {
        if (obj == nullptr) {
            ans += "\t" + str + "=()\n";
        } else {
            ans += "\t" + str + "=" + obj->ToString() + "\n";
        }
    }
    ans += "scopes:\n";
    for (auto [str, sc] : scopes_) {
        ans += "\t" + str + "=" + std::to_string(reinterpret_cast<std::uintptr_t>(sc)) + "\n";
    }
    return ans;
}
const std::map<std::string, Scope*>& Scope::GetScopes() const {
    return scopes_;
}
const std::map<std::string, Object*>& Scope::GetValues() const {
    return values_;
}
Scope* Scope::GetPrevios() const {
    return previos_;
}

Object* Lambda::Copy() const {
    static auto heap = GetHeap();
    auto ans = heap->Make<Lambda>();
    ans->arguments = arguments;
    ans->name = name;
    ans->SetScope(my_scope_);
    ans->SetCall(call_->Copy());
    return ans;
}
std::string Lambda::ToString() const {
    std::string ans = "<lambda '" + name + "' with args:";
    for (size_t i = 0; i < arguments.size(); ++i) {
        ans += " '" + arguments[i] + "'";
    }
    ans += ">";
    return ans;
}
Scope* Lambda::GetScope() const {
    return my_scope_;
}
Object* Lambda::GetCall() const {
    return call_;
}
void Lambda::SetScope(Scope* scope) {
    RemoveDependency(my_scope_);
    AddDependency(scope);
    my_scope_ = scope;
}
void Lambda::SetCall(Object* call) {
    RemoveDependency(call);
    AddDependency(call);
    call_ = call;
}
