#include <iostream>
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