#pragma once
#include <cstdlib>
#include <algorithm>
 
using namespace std;
 
template <typename Type>
class ArrayPtr {
public:
    ArrayPtr() = default;
 
    explicit ArrayPtr(size_t size) {
        if (size == 0) {
            raw_ptr_ = nullptr;
        }
        else {
            raw_ptr_ = new Type[size];
        }
    }
 
    explicit ArrayPtr(Type* raw_ptr) noexcept {
        raw_ptr_ = raw_ptr;
    }
 
    explicit ArrayPtr(ArrayPtr&& ptr) noexcept {
        if (raw_ptr_ != ptr.Get()) {
            raw_ptr_ = ptr.Release();
        }
    }
    ArrayPtr& operator=(ArrayPtr&& other) {
        raw_ptr_ = std::move(other.raw_ptr_);
       return  *this;
    }
 
    ArrayPtr(const ArrayPtr&) = delete;
 
    ~ArrayPtr() {
        delete[] raw_ptr_;
        raw_ptr_ = nullptr;
    }
 
    ArrayPtr& operator=(const ArrayPtr&) = delete;
 
    [[nodiscard]] Type* Release() noexcept {
        Type* raw_ptr = raw_ptr_;
        raw_ptr_ = nullptr;
        return raw_ptr;
    }
 
    Type& operator[](size_t index) noexcept {
        return *(raw_ptr_ + index);
    }
 
    const Type& operator[](size_t index) const noexcept {
        return *(raw_ptr_ + index);
    }
 
    explicit operator bool() const {
        if (raw_ptr_ == nullptr) {
            return false;
        }
        else {
            return true;
        }
    }
 
    Type* Get() const noexcept {
        return raw_ptr_;
    }
 
    void swap(ArrayPtr& other) noexcept {
        Type* temp = raw_ptr_;
        raw_ptr_ = other.raw_ptr_;
        other.raw_ptr_ = temp;
    }
 
private:
    Type* raw_ptr_ = nullptr;
};
