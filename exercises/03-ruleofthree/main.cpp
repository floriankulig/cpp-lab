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

  // Denkfrage: warum ist `new T[n]` fuer einen echten Vector untauglich?
  // `new T[n]` koppelt Allokation und Konstruktion zwangsweise: sobald der
  // Aufruf zurueckkommt, existieren bereits `n` fertig konstruierte
  // T-Objekte -- unabhaengig davon, wie viele `size_` gerade wirklich
  // "belegt" sind. Zwei Folgen: (1) T braucht zwingend einen Default-Ctor,
  // sonst kompiliert es gar nicht. (2) reserve(n) auf einem "echten" Vector
  // wuerde n Objekte konstruieren, die man danach beim push_back nur noch
  // ueberschreibt -- unnoetige Arbeit, bei teurem T auch unnoetige
  // Seiteneffekte. Loesung in Aufgabe 04: Allokation ueber
  // `::operator new(bytes)` (reserviert Speicher, konstruiert nichts) plus
  // gezieltes Placement-New nur fuer die Slots, die wirklich gebraucht
  // werden -> Allokation und Konstruktion sind zwei getrennte Schritte.
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
  ~Vector() {
    delete[] p_arr_;
    std::cout << "Vector gets destroyed." << std::endl;
  }

  // Copy-Ctor
  Vector(const Vector& other)
      : size_{other.size()}, capacity_{other.size()},
        p_arr_{new T[other.size()]} {
    std::cout << "Copy-Ctor called." << std::endl;
    for (std::size_t i = 0; i < size_; i++) {
      p_arr_[i] = other[i];
    }
  }

  // Copy-Assign
  Vector& operator=(const Vector& other) {
    std::cout << "Copy-Assgn-Ctor called." << std::endl;
    // Denkfrage: Selbstzuweisungen abfangen
    // Ohne den Check würde `delete[] p_arr_;` bei `v = v;` genau die
    // Ressourcen loeschen, aus denen `other` gleich kopieren soll (this ==
    // &other) -> use-after-free beim anschliessenden Kopieren.
    //
    // Denkfrage: Copy-and-Swap-Idiom
    // Alternative zu diesem manuellen delete+new: operator= nimmt den
    // Parameter BY VALUE (`operator=(Vector other)`), das erzwingt beim
    // Aufruf schon eine Kopie (bzw. bei rvalues einen Move -> Move-Assign
    // quasi gratis). Dann `swap(*this, other)`, das nur die drei Member
    // (size_, capacity_, p_arr_) tauscht -- kein Heap-Zugriff, kann nicht
    // werfen. Danach laeuft `other`s Destruktor am Funktionsende und gibt
    // die ALTEN Ressourcen frei.
    // Geloestes Problem: starke Exception-Garantie. Die einzige Stelle, an
    // der eine Exception fliegen kann (bad_alloc, T-Copy-Ctor wirft), ist
    // die Parameterkopie -- passiert komplett BEVOR `*this` angefasst wird.
    // Wirft es dort, ist `*this` unveraendert. Der swap selbst kann nicht
    // scheitern, weil dabei nichts alloziert wird.
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

  // Denkfrage:
  // bei `const Vector` nur read-zugriff
  const T& operator[](std::size_t idx) const { return p_arr_[idx]; }
  // Hier auch write Zugriff
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
  Vector<int> v1{};
  std::cout << "Capacity sollte 4 sein: "
            << (v1.capacity() == 4 ? "true" : "false") << std::endl;
  for (int i{0}; i < 5; i++) {
    v1.push_back(i + 1);
  }
  std::cout << "Capacity sollte 8 sein: "
            << (v1.capacity() == 8 ? "true" : "false") << std::endl;
  for (int i{5}; i < 10; i++) {
    v1.push_back(i + 1);
  }
  std::cout << "Capacity sollte 16 sein: "
            << (v1.capacity() == 16 ? "true" : "false") << std::endl;

  std::cout << "Should now call copy-ctor, because ref doesn't already exist:"
            << std::endl;
  Vector<int> ref = v1;
  std::cout << "Should now call copy-assign, because v1 already exists:"
            << std::endl;
  v1 = ref;

  for (auto& num : v1) {
    std::cout << num << ", ";
  }
  std::cout << "Range-based for funktioniert :)" << std::endl;
  const Vector<int> v2{2, 69};
  std::cout << v2[0] << std::endl;
  return 0;
}