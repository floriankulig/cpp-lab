#include "Tracer.cpp"
#include "iostream"
#include <type_traits>
#include <utility>
#include <vector>

template <typename T> class UniquePtr {
  T* ptr_;

public:
  UniquePtr() : ptr_{nullptr} {}
  explicit UniquePtr(T* p) : ptr_{p} {}

  ~UniquePtr() {
    std::cout << "uniq destr" << std::endl;
    delete ptr_;
  }

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
  void swap(UniquePtr<T>& other) noexcept { std::swap(ptr_, other.ptr_); }
};

template <typename T, typename... Args>
UniquePtr<T> MakeUnique(Args&&... args) {
  return UniquePtr<T>{new T(std::forward<Args>(args)...)};
}

struct Obj {
  int a{12};
  int b{24};

  Obj() = default;
  Obj(int ia, int ib) : a(ia), b(ib) {}
  ~Obj() { std::cout << "Obj deleted " << a << std::endl; }
};

static_assert(!std::is_copy_constructible_v<UniquePtr<int>>,
              "UniquePtr kann nicht kopiert werden");
static_assert(std::is_move_constructible_v<UniquePtr<int>>,
              "UniquePtr kann gemoved werden");
static_assert(std::is_nothrow_move_constructible_v<UniquePtr<int>>,
              "UniquePtr kann gemoved ohne Risiko zu throwen werden");
static_assert(std::is_nothrow_move_assignable_v<UniquePtr<int>>,
              "UniquePtr kann gemoved ohne Risiko zu throwen werden");

void testObj() {
  Obj* raw = new Obj{};
  UniquePtr<Obj> expl{raw};
  auto ptr = MakeUnique<Obj>(45, 67);
  std::cout << "Hello World!" << std::endl;
  std::cout << ptr.get() << std::endl;
  auto newPtr = std::move(ptr);
  newPtr = std::move(newPtr);
  std::cout << ptr.get() << std::endl;
  std::cout << newPtr.get() << std::endl;
  std::cout << "== Explicit ==" << std::endl;
  std::cout << raw << std::endl;
  std::cout << expl.get() << std::endl;
}

using TracerPtr = UniquePtr<Tracer>;

void testTracer() {
  auto tPtr = MakeUnique<Tracer>();
  {
    std::cout << "Scope start" << std::endl;
    auto scoped = MakeUnique<Tracer>();
    auto t = new Tracer{};
    // scoped2 takes ownership and destructs
    UniquePtr<Tracer> scoped2{t};
    std::cout << "Scope end" << std::endl;
  }
  auto scndTPtr = MakeUnique<Tracer>();
  scndTPtr->SetId(69);
  tPtr.swap(scndTPtr);
}

int main() {
  std::cout << "=== Obj ===" << std::endl;
  testObj();
  std::cout << "=== Tracer ===" << std::endl;
  testTracer();

  std::cout << "=== Tracer Vector ===" << std::endl;
  std::vector<TracerPtr> v;
  std::cout << "sizeof(TracerPtr): " << sizeof(TracerPtr) << std::endl;
  v.reserve(2);
  v.push_back(MakeUnique<Tracer>());
  v.push_back(MakeUnique<Tracer>());
  std::cout << "First loop" << std::endl;
  for (auto& tracPtr : v) {
    std::cout << tracPtr.get() << std::endl;
  }
  v.push_back(MakeUnique<Tracer>());
  std::cout << "Second loop" << std::endl;
  for (auto& tracPtr : v) {
    std::cout << tracPtr.get() << std::endl;
  }

  return 0;
}