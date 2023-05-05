#pragma once

#include <iostream>
#include <cassert>
#include <initializer_list>
#include <utility>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t new_capacity) : new_capacity_(new_capacity) {}

    size_t GetCapacity() const {
        return new_capacity_;
    }

private:
    size_t new_capacity_ = 0;
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

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : v_(size) {
        capacity_ = size_ = size;

        if (size > 0) {
            std::fill(begin(), end(), Type());
        }
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : v_(size) {
        capacity_ = size_ = size;

        if (size > 0) {
            std::fill(begin(), end(), value);
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : v_(init.size()), size_(init.size()), capacity_(init.size()) {
        if (capacity_ > 0) {
            std::copy(init.begin(), init.end(), v_.Get());
        }
    }

    SimpleVector(const SimpleVector& other) {
        try {
            size_ = other.GetSize();
            capacity_ = other.GetCapacity();

            ArrayPtr<Type> temp_v(capacity_);

            std::copy(other.begin(), other.end(), temp_v.Get());

            for (auto it = temp_v.Get() + size_; it != temp_v.Get() + capacity_; ++it) {
                *it = std::move(Type());
            }

            v_.swap(temp_v);
        }
        catch (const std::bad_alloc&) {
            capacity_ = size_ = 0;
            throw;
        }
    }

    // Создаёт вектор ...
    SimpleVector(SimpleVector&& v) noexcept : size_(std::exchange(v.size_, 0)), capacity_(std::exchange(v.capacity_, 0)) {
        v_.swap(v.v_);
    }

    SimpleVector(const ReserveProxyObj& obj) : capacity_(obj.GetCapacity()) {
        if (capacity_ > 0) {
            std::fill(begin(), end(), Type());
        }
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            auto rhs_copy(rhs);
            swap(rhs_copy);
        }

        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            v_.swap(rhs.v_);
            size_ = std::exchange(rhs.size_, 0);
            capacity_ = std::exchange(rhs.capacity_, 0);
        }

        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return (GetSize() == 0);
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return v_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return v_[index];
    }
 
    Type& At(size_t index) {
        return (index >= size_) ? throw std::out_of_range("out_of_range") : v_[index];
    }

  
    const Type& At(size_t index) const {
        return (index >= size_) ? throw std::out_of_range("out_of_range") : v_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }
        else if (new_size <= capacity_) {
            for (auto it = begin() + size_; it != begin() + new_size; ++it) {
                *it = std::move(Type());
            }

            size_ = new_size;
        }
        else {
            ArrayPtr<Type> temp_v(new_size);

            for (auto it = temp_v.Get(); it != temp_v.Get() + size_; ++it) {
                *it = std::move(*(begin() + (it - temp_v.Get())));
            }

            for (auto it = temp_v.Get() + size_; it != temp_v.Get() + new_size; ++it) {
                *it = std::move(Type());
            }

            v_.swap(temp_v);

            capacity_ = size_ = new_size;
        }
    }

    Iterator begin() noexcept {
        return Iterator(v_.Get());
    }
 
    Iterator end() noexcept {
        return Iterator(v_.Get() + size_);
    }

    ConstIterator begin() const noexcept {
        return ConstIterator(v_.Get());
    }
 
    ConstIterator end() const noexcept {
        return ConstIterator(v_.Get() + size_);
    }

    ConstIterator cbegin() const noexcept {
        return ConstIterator(v_.Get());
    }

    ConstIterator cend() const noexcept {
        return ConstIterator(v_.Get() + size_);
    }

    void PushBack(const Type& item) {
        if (size_ < capacity_) {
            v_[size_++] = item;
        }
        else {
            ArrayPtr<Type> temp_v((capacity_ != 0) ? 2 * capacity_ : 1);

            std::copy(begin(), end(), temp_v.Get());
            std::fill(temp_v.Get() + size_, temp_v.Get() + capacity_, Type());
            temp_v[size_++] = item;

            v_.swap(temp_v);

            capacity_ = (capacity_ != 0) ? 2 * capacity_ : 1;
        }
    }

    void PushBack(Type&& item) {
        if (size_ < capacity_) {
            v_[size_++] = std::move(item);
        }
        else {
            ArrayPtr<Type> temp_v((capacity_ != 0) ? 2 * capacity_ : 1);

            for (size_t i = 0; i < size_; ++i) {
                temp_v[i] = std::move(v_[i]);
            }

            for (size_t i = size_; i < capacity_; ++i) {
                temp_v[i] = std::move(Type());
            }

            temp_v[size_++] = std::move(item);

            v_.swap(temp_v);

            capacity_ = (capacity_ != 0) ? 2 * capacity_ : 1;
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
            auto number = pos - v_.Get();
            ArrayPtr<Type> temp_v((capacity_ != 0) ? 2 * capacity_ : 1);

            std::copy(begin(), end(), temp_v.Get());
            std::fill(temp_v.Get() + size_, temp_v.Get() + capacity_, Type());
            std::copy_backward(temp_v.Get() + number, temp_v.Get() + capacity_, temp_v.Get() + capacity_ + 1);

            temp_v[number] = value;

            v_.swap(temp_v);

            ++size_;
            capacity_ = (capacity_ != 0) ? 2 * capacity_ : 1;

            return Iterator(v_.Get() + number);
        }
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        if (size_ < capacity_) {
            auto it = Iterator(pos);

            for (int i = static_cast<int>(end() - begin() + 1); i > Iterator(pos) - begin(); --i) {
                v_[i] = std::move(v_[i - 1]);
            }

            v_[it - begin()] = std::move(value);
            ++size_;

            return Iterator(pos);
        }
        else {
            auto number = pos - v_.Get();
            ArrayPtr<Type> temp_v((capacity_ != 0) ? 2 * capacity_ : 1);

            for (size_t i = 0; i < size_; ++i) {
                temp_v[i] = std::move(v_[i]);
            }

            for (size_t i = size_; i < capacity_; ++i) {
                temp_v[i] = std::move(Type());
            }

            for (int i = static_cast<int>(capacity_ + 1); i > number; --i) {
                temp_v[i] = std::move(temp_v[i - 1]);
            }

            temp_v[number] = std::move(value);

            v_.swap(temp_v);

            ++size_;
            capacity_ = (capacity_ != 0) ? 2 * capacity_ : 1;

            return Iterator(v_.Get() + number);
        }
    }

 
    void PopBack() noexcept {
        --size_;
    }

  
    Iterator Erase(ConstIterator pos) {
        auto aim = std::distance(cbegin(), pos);
        if (pos == begin() + size_) {   
            size_--;                                                                   
        }

        else {
            ArrayPtr<Type> temp(size_);
            temp[size_ - 1] = std::move(Type{});
          //  auto aim = pos - begin();
            for (auto i = 0; i < aim; ++i) {
                temp[i] = std::move(v_[i]);
            }
            for (auto i = static_cast<int>(size_ - 1); i > aim; --i) {
                temp[i - 1] = std::move(v_[i]);
            }
            v_.swap(temp);
            size_--;
        }
        return Iterator(cbegin() + aim);
    }


    void swap(SimpleVector& other) noexcept {
        v_.swap(other.v_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (capacity_ < new_capacity) {
            ArrayPtr<Type> temp_v(new_capacity);

            std::copy(begin(), end(), temp_v.Get());
            std::fill(temp_v.Get() + size_, temp_v.Get() + new_capacity, Type());

            v_.swap(temp_v);

            capacity_ = new_capacity;
        }
    }

private:
    ArrayPtr<Type> v_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs.GetSize() == rhs.GetSize()) ?
        std::equal(lhs.begin(), lhs.end(), rhs.begin()) : false;
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs.GetSize() != rhs.GetSize()) ? true :
        !std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
