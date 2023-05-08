#pragma once
 
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <cassert>
#include <iterator>
#include "array_ptr.h"
 
class ReserveProxyObj {
public:    
    explicit ReserveProxyObj (size_t capacity_to_reserve) : capacity_(capacity_to_reserve) {}
    size_t Reserve_capacity() {
        return capacity_;
    }  
private:    
    size_t capacity_;
};
 
ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}
 
template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;
 
    SimpleVector() noexcept = default;
 
    explicit SimpleVector(size_t size) : SimpleVector(size, std::move(Type{})) {}
 
    SimpleVector(size_t size, const Type& value) : simple_vector_(size), size_(size), capacity_(size) {
        std::fill(begin(), end(), value);
    }
 
    SimpleVector(std::initializer_list<Type> init) : simple_vector_(init.size()), size_(init.size()), capacity_(init.size()){
std::copy(init.begin(), init.end(), simple_vector_.Get());
    }
    
    SimpleVector(ReserveProxyObj capacity_to_reserve) {
        Reserve(capacity_to_reserve.Reserve_capacity());
    }
 
    size_t GetSize() const noexcept {
        return size_;
    }
 
    size_t GetCapacity() const noexcept {
        return capacity_;
    }
 
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }
 
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return simple_vector_[index];
    }
 
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return simple_vector_[index];
    }
 
    Type& At(size_t index) {
        if (index < size_) {
            return simple_vector_[index];
        } else {
            throw std::out_of_range("Non-existent vector element.");
        }
    }
 
    const Type& At(size_t index) const {
        if (index < size_) {
            return simple_vector_[index];
        } else {
            throw std::out_of_range("Non-existent vector element.");
        }
    }
 
    void Clear() noexcept {
        size_ = 0;
    }
 
    void Resize(size_t new_size) {
        if (size_ >= new_size) {
            size_ = new_size;
            return;
        } else if (size_< new_size && capacity_ > new_size) {
            for (auto iter = begin() + new_size; iter != end(); --iter) {
                *iter = std::move(Type());
    }
            size_ = new_size;
            return;
        } else {
            ArrayPtr<Type> helper(new_size);
            std::move(begin(), end(), &helper[0]);
            simple_vector_.swap(helper);
            size_ = new_size;
            capacity_ = new_size*2;
        }
    }
 
    Iterator begin() noexcept {
        return Iterator(simple_vector_.Get());
    }
 
    Iterator end() noexcept {
        return Iterator(simple_vector_.Get() + size_);  
    }
 
    ConstIterator begin() const noexcept { 
        return ConstIterator(simple_vector_.Get());
    }
 
    ConstIterator end() const noexcept {
        return ConstIterator(simple_vector_.Get() + size_);
    }
 
    ConstIterator cbegin() const noexcept {
        return ConstIterator(simple_vector_.Get());
    }
 
    ConstIterator cend() const noexcept {
        return ConstIterator(simple_vector_.Get() + size_);
    }
    
    SimpleVector(const SimpleVector& other) {
        ArrayPtr<Type> helper(other.size_);
        size_ = other.size_;
        capacity_= other.capacity_;
        std::copy(other.begin(), other.end(), &helper[0]);
        simple_vector_.swap(helper);
    }
    
    SimpleVector(SimpleVector&& other) : simple_vector_(other.size_) {
        size_ = std::move(other.size_);
        simple_vector_.swap(other.simple_vector_);
        other.Clear();
    }
 
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector helper(rhs);
            swap(helper);
        } 
        return *this;
    }
    
    void PushBack(const Type& item) {
        if (capacity_ > size_) {
            simple_vector_[size_++] = item;
            return;
        } else {
            ArrayPtr<Type> helper(capacity_*=2);
            std::copy(begin(), end(), helper.Get());     
           std::fill(helper.Get() + size_, helper.Get() + capacity_, Type()); 
            simple_vector_.swap(helper);            
            simple_vector_[size_++] = item;
            
      capacity_ = capacity_*=2;
        }
    }
 
    void PushBack(Type&& item) {
        if (capacity_ > size_) {
            simple_vector_[size_++] = std::move(item);
            return;
        }
        if (capacity_) {
            ArrayPtr<Type> helper(capacity_*=2);
            std::move(begin(), end(), helper.Get());
            simple_vector_.swap(helper);
            simple_vector_[size_++] = std::move(item);
        } else {
            ArrayPtr<Type> helper(++capacity_);
            std::move(begin(), end(), helper.Get());
            simple_vector_.swap(helper);
            simple_vector_[size_++] = std::move(item);
            return;
        }
    }
 
Iterator Insert(ConstIterator pos, const Type& value) {
      if (size_ < capacity_) {
            Iterator it = Iterator(pos);
            std::copy_backward(Iterator(pos), end(), end() + 1);
            *it = value;
            ++size_;
            return Iterator(pos);
        }
        else {
            auto num = pos - simple_vector_.Get();
            ArrayPtr<Type> helper(capacity_*=2);
   std::copy(begin(), end(),  helper.Get());
  std::fill( helper.Get() + size_, helper.Get() + capacity_, Type());
  std::copy_backward(helper.Get() + num, helper.Get() + capacity_, helper.Get() + capacity_ + 1);
           helper[num] = value;
           simple_vector_.swap(helper);
            ++size_;
            capacity_ = capacity_*=2;
            return Iterator(simple_vector_.Get() + num);
        }
    }
  
Iterator Insert(ConstIterator pos, Type&& value) {
          size_t new_element = pos - simple_vector_.Get();
        if (size_ >= capacity_) {
          size_t new_capacity = capacity_ == 0 ? 1 : 2 * capacity_;
            ArrayPtr<Type> helper(new_capacity);
            std::generate(helper.Get(), helper.Get() + (capacity_*=2), []() { return Type(); });
            std::move(simple_vector_.Get(), const_cast<Iterator>(pos), helper.Get());
            helper[new_element] = std::move(value);
            std::move(const_cast<Iterator>(pos), simple_vector_.Get() + size_, helper.Get() + new_element + 1);
            simple_vector_.swap(helper);
            capacity_ =  new_capacity;
        } else {
   std::move_backward(const_cast<Iterator>(pos), simple_vector_.Get() + size_, simple_vector_.Get() + size_ + 1);
            simple_vector_[new_element] = std::move(value);
        }
        ++size_;
        return Iterator(simple_vector_.Get() + new_element);
    }

     void PopBack() noexcept {
       simple_vector_[size_ - 1] = Type();
        --size_;
    }
 
    Iterator Erase(ConstIterator pos) {
            auto index = std::distance(cbegin(), pos);
            std::move(&simple_vector_[index + 1], end(), const_cast<Iterator>(pos));
            --size_;
            return const_cast<Iterator>(pos);   
    }
 
    void swap(SimpleVector& other) noexcept {
        simple_vector_.swap(other.simple_vector_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    
   void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> helper(new_capacity);
            std::fill(helper.Get(), helper.Get() + capacity_, Type());
            std::copy(simple_vector_.Get(), simple_vector_.Get() + size_, helper.Get());

            simple_vector_.swap(helper);
            capacity_ = new_capacity;
        }
    }  
        
private:
    ArrayPtr<Type> simple_vector_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};
 
template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
 
template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}
 
template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
 
template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs < rhs || lhs == rhs;
}
 
template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}
 
template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
