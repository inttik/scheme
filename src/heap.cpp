#include "heap.h"
#include <memory>
#include "object.h"

Heap::Heap() {
    root_ = Make<Empty>();
}
void Heap::AddRootDependency(Object* object) {
    root_->AddDependency(object);
}
void Heap::RemoveRootDependencty(Object* object) {
    root_->RemoveDependency(object);
}
void Heap::DeleteUnuse() {
    for (auto& obj : objects_) {
        obj->mark_bit_ = false;
    }
    MarkDfs(root_);
    for (size_t i = 0; i < objects_.size();) {
        if (objects_[i]->mark_bit_ == true) {
            objects_[i]->mark_bit_ = false;
            ++i;
            continue;
        }
        swap(objects_[i], objects_.back());
        ++dealloc_count;
        objects_.pop_back();
    }
}
void Heap::MarkDfs(Object* root) {
    root->mark_bit_ = true;
    for (auto& other : root->dependency_) {
        if (other->mark_bit_ == true) {
            continue;
        }
        MarkDfs(other);
    }
}

Heap* GetHeap() {
    static Heap heap;
    return &heap;
}
