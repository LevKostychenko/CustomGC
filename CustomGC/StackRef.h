#pragma once
#include "GCHeader.h"
#include <vector>

std::vector<GCHeader**> stck_references;

template <class T>
class StackRef {
public:
    T* ref;
    StackRef() {
        ref = nullptr;
        stck_references.push_back(reinterpret_cast<GCHeader**>(&ref));
    }

    ~StackRef() {
        stck_references.pop_back();
    }
};