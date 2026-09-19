#include "iostream"
#include <memory>
#include <utility>

template <typename T> class UniquePtr {
  T* ptr_;

public:
  UniquePtr() : ptr_{nullptr} {}
  explicit UniquePtr(T* p) : ptr_{p} {}

  ~UniquePtr() { delete ptr_; }

  // Copy-Ctor
  UniquePtr(const UniquePtr<T>& other) = delete;
  // Copy-Assign
  UniquePtr<T> operator=(const UniquePtr<T>& rhs) = delete;

  // Move-Ctor
  UniquePtr(UniquePtr<T>&& other) noexcept : ptr_{other.ptr_} {
    other.ptr_ = nullptr;
  };
  // Move-Assign
  UniquePtr& operator=(UniquePtr<T>&& rhs) noexcept {
    if (this != &rhs) {
      delete ptr_;
      ptr_ = std::exchange(rhs.ptr_, nullptr);
    }
    return *this;
  };

  T& operator*() const noexcept { return *ptr_; }
  T* operator->() const noexcept { return ptr_; }
  explicit operator bool() const noexcept { return ptr_ != nullptr; };

  T* get() const { return ptr_; }
  [[nodiscard]] T* release() { return std::exchange(ptr_, nullptr); }
  void reset(T* p = nullptr) {
    delete ptr_;
    ptr_ = p;
  }
  void swap(UniquePtr<T> other) noexcept { std::swap(ptr_, other.ptr_); }
};

template <typename T, typename... Args>
UniquePtr<T> MakeUnique(Args&&... args) {
  return UniquePtr<T>{new T{std::forward<Args>(args)...}};
}

struct Obj {
  int a{12};
  int b{24};

  ~Obj() { std::cout << "Obj deleted " << a << std::endl; }
};

int main() {
  Obj testobj{67, 69};
  auto testobj2 = new Obj{6767, 69};
  auto ptr = std::make_unique<Obj>();
  UniquePtr<Obj> myptr{testobj2};
  std::cout << "Hello World!" << std::endl;
  return 0;
}