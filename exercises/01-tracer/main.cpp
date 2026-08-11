#include <algorithm>
#include <iostream>
#include <ostream>
#include <utility>
#include <vector>

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

void funcRef(const Tracer& tracer) { tracer.GetId(); }

void funcVal(Tracer tracer) { tracer.GetId(); }

Tracer funcRet() { return Tracer{}; }

int main() {
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "1." << std::endl;
  // Normaler Konstruktor.
  Tracer t{};

  std::cout << "2." << std::endl;
  // Gar nichts.
  funcRef(t);
  std::cout << "Jetzt by value" << std::endl;
  // Copy constructor
  funcVal(t);

  std::cout << "3." << std::endl;
  // Move constructor
  // Korrektur: Hier ist es anscheinend der normale Constructor. Warum? Wird das
  // vielleicht vom Compiler geinlined? Und einfach direkt über den normalen
  // Constructor dann aufgelöst.
  Tracer retTracer = funcRet();

  std::cout << "4." << std::endl;
  // Hier würde ich sagen gar nichts, da der Vektor bei Pushback einfach nur die
  // References auf den Tracer nimmt.
  std::vector<Tracer> v;
  v.push_back(t);

  std::cout << "5." << std::endl;
  // Eigentlich aus der Intuition auch nichts, aber das move wird ja irgendwas
  // machen also rate ich mal dass der move constructor gecallt wird
  v.push_back(std::move(t));

  std::cout << "6." << std::endl;
  // Hier habe ich überhaupt gar keine Ahnung, was im Placeback macht. Aber es
  // wird auf jeden Fall der Default Constructor aufgerufen.
  v.emplace_back();

  std::cout << "7." << std::endl;
  Tracer a;
  // Hier erwarte ich eigentlich auch gar nichts. weil das wieder References
  // sind.
  v.push_back(a);
  v.push_back(a);
  v.push_back(a);
  v.push_back(a);
  v.push_back(a);

  std::cout << "8." << std::endl;
  // Hier erwarte ich eigentlich auch gar nichts. weil das wieder References
  // sind. Das reserviert ja nur Speicher, aber die Objekte Referenzen sind
  // davon, glaube ich, nicht betroffen.
  std::vector<Tracer> v2;
  v2.reserve(5);
  v2.push_back(a);
  v2.push_back(a);
  v2.push_back(a);
  v2.push_back(a);
  v2.push_back(a);

  std::cout << "9." << std::endl;
  // Hier erwarte ich, dass der Move Assignment Constructor aufgerufen wird.
  // Natürlich nachdem der Default-Constructor für das Objekt B durchgelaufen
  // ist.
  const Tracer b{};
  auto c = std::move(b);

  std::cout << "10." << std::endl;
  // Hier erwarte ich zweimal also jeweils einen Move-Constructor, weil Swap
  // bestimmt mit einer temporären Variable arbeitet und diese einfach nur hin
  // und her moved.
  Tracer d{};
  std::swap(d, c);
  std::cout << "STACK AUFRÄUMEN" << std::endl;
  return 0;
}