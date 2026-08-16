#include <algorithm>
#include <cstddef>
#include <iostream>
#include <stdexcept>

template <typename T> class Vector {
private:
  const static std::size_t BASE_CAPACITY_ = 4;
  std::size_t size_;
  std::size_t capacity_;
  T* p_arr_;

  void grow() {
    capacity_ *= 2;
    if (capacity_ == 0) {
      capacity_ = BASE_CAPACITY_;
    }
    T* new_arr = new T[capacity_];
    for (std::size_t i = 0; i < size_; i++) {
      new_arr[i] = std::move(p_arr_[i]);
    }
    delete[] p_arr_;
    p_arr_ = new_arr;
  }

public:
  Vector()
      : size_(0), capacity_(BASE_CAPACITY_), p_arr_{new T[BASE_CAPACITY_]} {}

  Vector(std::size_t size, const T& value)
      : size_(size), capacity_(size), p_arr_{new T[size]} {
    for (std::size_t i = 0; i < size; i++) {
      p_arr_[i] = value;
    }
  }
  ~Vector() { delete[] p_arr_; }

  // Copy-Ctor
  Vector(const Vector& other)
      : size_{other.size()}, capacity_{other.size()},
        p_arr_{new T[other.size()]} {
    for (std::size_t i = 0; i < size_; i++) {
      p_arr_[i] = other[i];
    }
  }

  // Copy-Assign
  Vector& operator=(const Vector& other) {
    if (this != &other) {
      size_ = other.size();
      capacity_ = other.size();

      delete[] p_arr_;
      p_arr_ = new T[other.size()];
      for (std::size_t i = 0; i < size_; i++) {
        p_arr_[i] = other[i];
      }
    }

    return *this;
  }

  const T& operator[](std::size_t idx) const { return p_arr_[idx]; }

  T& operator[](std::size_t idx) { return p_arr_[idx]; }

  std::size_t size() const { return size_; }
  std::size_t capacity() const { return capacity_; }
  bool empty() const { return size_ == 0; }

  T* begin() { return &p_arr_[0]; }
  const T* begin() const { return &p_arr_[0]; }
  T* end() { return &p_arr_[size_]; }
  const T* end() const { return &p_arr_[size_]; }

  T& at(std::size_t idx) {
    if (idx >= size_) {
      throw std::out_of_range{"Cannot access Index out of bounds"};
    }
    return p_arr_[idx];
  }
  const T& at(std::size_t idx) const {
    if (idx >= size_) {
      throw std::out_of_range{"Cannot access Index out of bounds"};
    }
    return p_arr_[idx];
  }

  void push_back(const T& value) {
    if (size_ == capacity_) {
      grow();
    }
    p_arr_[size_] = value;
    size_ += 1;
  }
};

int main() {
  std::cout << "Hello World" << std::endl;
  return 0;
}