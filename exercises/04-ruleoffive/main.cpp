#include <cstddef>
#include <ios>
#include <iostream>
#include <new>
#include <stdexcept>
#include <utility>

class Tracer {
private:
  inline static int runningId = 1;
  int id_;

public:
  int GetId() const { return id_; }
  void SetId(int newId) { id_ = newId; }

  Tracer() {
    id_ = runningId++;
    std::cout << "(Constructor) " << "Objekt mit ID: " << id_ << std::endl;
  };

  ~Tracer() {
    std::cout << "(Destructor) " << "Objekt mit ID: " << id_ << std::endl;
  };

  Tracer(const Tracer& otherTracer) {
    id_ = runningId++;
    std::cout << "(Copy-Constructor) " << "Objekt mit ID: " << id_
              << " von Objekt #" << otherTracer.GetId() << std::endl;
  }

  Tracer(Tracer&& otherTracer) noexcept {
    id_ = otherTracer.GetId();
    std::cout << "(Move-Constructor) " << "Objekt mit ID: " << id_
              << " von Objekt #" << otherTracer.GetId() << std::endl;
    otherTracer.SetId(-1 * otherTracer.GetId());
  };

  Tracer& operator=(const Tracer& otherTracer) {
    if (this != &otherTracer) {
      id_ = runningId++;
    }

    std::cout << "(Copy-Assign-Constructor) " << "Objekt mit ID: " << id_
              << " von Objekt #" << otherTracer.GetId() << std::endl;
    return *this;
  }

  Tracer& operator=(Tracer&& otherTracer) noexcept {
    if (this != &otherTracer) {
      id_ = otherTracer.GetId();
      std::cout << "(Move-Assign-Constructor) " << "Objekt mit ID: " << id_
                << " von Objekt #" << otherTracer.GetId() << std::endl;
      otherTracer.SetId(-1 * otherTracer.GetId());
      // delete von otherTracer passiert automatisch
    }

    return *this;
  }
};

using Size = std::size_t;
using Capacity = std::size_t;

template <typename T> class Vector {
  static constexpr Capacity BASE_CAPACITY = 4;
  Size size_;
  Capacity capacity_;
  T* pMem_;

  void growTo(Capacity newCap) {
    T* oldPMem = pMem_;
    if (newCap == 0) {
      newCap = BASE_CAPACITY;
    }
    capacity_ = newCap;
    pMem_ = static_cast<T*>(::operator new(capacity_ * sizeof(T)));

    // Move from old memory
    for (std::size_t i = 0; i < size_; i++) {
      new (pMem_ + i) T(std::move_if_noexcept(oldPMem[i]));
    }

    // Free old memory/resources
    for (std::size_t i = 0; i < size_; i++) {
      oldPMem[i].~T();
    }
    ::operator delete(oldPMem);
  }

  void grow() { growTo(capacity_ * 2); }

public:
  Vector() : size_{0}, capacity_{0}, pMem_{nullptr} {}
  Vector(std::size_t count, const T& value)
      : size_{count}, capacity_{count},
        pMem_{static_cast<T*>(::operator new(capacity_ * sizeof(T)))} {
    for (std::size_t i = 0; i < size_; i++) {
      new (pMem_ + i) T(value);
    }
  }
  ~Vector() {
    for (std::size_t i = 0; i < size_; i++) {
      pMem_[i].~T();
    }
    ::operator delete(pMem_);
  }

  // Copy-Ctor
  Vector(const Vector<T>& rhs)
      : size_{rhs.size_}, capacity_{rhs.capacity_},
        pMem_{static_cast<T*>(::operator new(capacity_ * sizeof(T)))} {
    for (std::size_t i = 0; i < size_; i++) {
      new (pMem_ + i) T(rhs[i]);
    }
  }
  // Move-Ctor
  Vector(Vector<T>&& rhs) noexcept
      : size_{rhs.size_}, capacity_{rhs.capacity_}, pMem_{rhs.pMem_} {
    rhs.size_ = 0;
    rhs.capacity_ = 0;
    rhs.pMem_ = nullptr;
  }

  // Copy-Assign
  Vector& operator=(const Vector<T>& rhs) {
    if (this != &rhs) {
      // Free old memory/resources
      for (std::size_t i = 0; i < size_; i++) {
        pMem_[i].~T();
      }
      ::operator delete(pMem_);

      size_ = rhs.size_;
      capacity_ = rhs.capacity_;

      pMem_ = static_cast<T*>(::operator new(capacity_ * sizeof(T)));
      for (std::size_t i = 0; i < size_; i++) {
        new (pMem_ + i) T(rhs[i]);
      }
    }
    return *this;
  }
  // Move-Assign
  Vector& operator=(Vector<T>&& rhs) noexcept {
    if (this != &rhs) {
      // Free old memory/resources
      for (std::size_t i = 0; i < size_; i++) {
        pMem_[i].~T();
      }
      ::operator delete(pMem_);

      size_ = std::exchange(rhs.size_, 0);
      capacity_ = std::exchange(rhs.capacity_, 0);
      pMem_ = std::exchange(rhs.pMem_, nullptr);
    }
    return *this;
  }

  const T& operator[](std::size_t idx) const { return pMem_[idx]; }
  T& operator[](std::size_t idx) { return pMem_[idx]; }

  const T& at(std::size_t idx) const {
    if (idx >= size_) {
      throw std::out_of_range("Cannot access out of range indices");
    }
    return pMem_[idx];
  }
  T& at(std::size_t idx) {
    if (idx >= size_) {
      throw std::out_of_range("Cannot access out of range indices");
    }
    return pMem_[idx];
  }

  Size size() const { return size_; }
  Capacity capacity() const { return capacity_; }
  bool empty() { return size_ == 0; }

  void reserve(Size n) {
    std::cout << "Vector.reserve()" << std::endl;
    if (n <= capacity_) {
      return;
    }

    T* oldPMem = pMem_;
    capacity_ = n;
    pMem_ = static_cast<T*>(::operator new(capacity_ * sizeof(T)));

    // Move from old memory
    for (std::size_t i = 0; i < size_; i++) {
      new (pMem_ + i) T(std::move_if_noexcept(oldPMem[i]));
    }

    // Free old memory/resources
    for (std::size_t i = 0; i < size_; i++) {
      oldPMem[i].~T();
    }
    ::operator delete(oldPMem);
  }

  void clear() {
    // Free old memory/resources
    for (std::size_t i = 0; i < size_; i++) {
      pMem_[i].~T();
    }
    size_ = 0;
  }

  void push_back(const T& value) {
    if (size_ == capacity_) {
      grow();
    }
    new (pMem_ + size_) T(value);
    size_ += 1;
  }

  template <typename... Args> T& emplace_back(Args&&... args) {
    if (size_ == capacity_) {
      grow();
    }
    new (pMem_ + size_) T(std::forward<Args>(args)...);
    size_ += 1;
    return pMem_[size_ - 1];
  }

  T pop_back() {
    std::cout << "Vector.pop_back()" << std::endl;
    size_ -= 1;
    auto last = std::move(pMem_[size_]);
    pMem_[size_].~T();

    return last;
  }
};

struct Obj {
  Obj() { std::cout << "Obj gets constructed" << std::endl; }
  ~Obj() { std::cout << "Obj gets destroyed" << std::endl; }
};

int main() {
  Vector<Tracer> v1;
  // Konstruiert nichts
  v1.reserve(4);

  for (auto i = 0; i < 5; i++) {
    v1.push_back(Tracer{});
  }
  std::cout << "nach 5 push_backs" << std::endl;

  Vector<Tracer> v3 = v1;

  v1.clear();
  std::cout << "nach clear" << std::endl;

  std::cout << "MOVE" << std::endl;
  Vector<Tracer> v2 = std::move(v3);
  std::cout << "size v1 " << v1.size() << std::endl;
  std::cout << "size v2 " << v2.size() << std::endl;
  std::cout << "size v3 " << v3.size() << std::endl;

  // push_back möglich after move
  v3.push_back(Tracer{});
  std::cout << "v3 size " << v3.size() << " ist 1" << std::endl;
  // emplace ruft nur 1 Konstruktor auf (console)
  v3.emplace_back();

  Tracer ref = v3.pop_back();

  return 0;
}