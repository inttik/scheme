#pragma once

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "object.h"

class Heap {
private:
    Heap();

public:
    inline static int alloc_count;
    inline static int dealloc_count;
    template <class ObjectType, typename... Args>
    requires std::is_base_of_v<Object, ObjectType> ObjectType* Make(Args&&... args) {
        auto current = new ObjectType(std::forward<Args>(args)...);
        ++alloc_count;
        objects_.push_back(std::unique_ptr<Object>(current));
        return current;
    }
    void AddRootDependency(Object* object);
    void RemoveRootDependencty(Object* object);
    void DeleteUnuse();

private:
    friend Heap* GetHeap();

    static void MarkDfs(Object* root);

    std::vector<std::unique_ptr<Object>> objects_;
    Object* root_;
};

Heap* GetHeap();
