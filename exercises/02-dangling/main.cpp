#include <array>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

int& localRef() {
  int number = 1;
  return number;
}
// ASan: SUMMARY: AddressSanitizer: stack-use-after-return main.cpp:26 in main
// number lebt nur im Stack Frame von localRef(). Nach dem return ist der
// Frame weg, test_num referenziert toten Stack-Speicher.
// Warum warnt der Compiler hier, aber nicht bei Fall 3? -Wreturn-stack-address
// ist ein rein syntaktisches Pattern-Match auf "return einer Adresse/Referenz
// eines benannten lokalen Objekts" direkt an der return-Zeile. Ob ein vector
// reallociert, ist dagegen ein Laufzeitzustand (size() vs. capacity()), den
// der Compiler nicht statisch kennt.

std::string getString() { return "somestring"; }

int main() {
  // Fall 1:
  int& test_num = localRef();
  std::cout << test_num << std::endl;

  // Fall 2:
  // getString() liefert ein Temporary (std::string), das am Ende dieses
  // Full-Expressions stirbt. string_view besitzt nichts, sie haelt nur einen
  // Datenzeiger + Laenge auf fremde Daten -> zeigt danach ins Nichts.
  // Ein std::string statt string_view haette das Problem nicht: der
  // Copy-/Move-Ctor kopiert den Inhalt in einen eigenen, von s selbst
  // verwalteten Buffer, unabhaengig vom Temporary.
  std::string_view sv = getString();
  std::cout << sv << std::endl;

  // das hier funktioniert weil hier nicht nur zwei pointer auf die anfangs
  // und-ad Endressen von der String View sind, sondern das gesamte
  // String-Objekt mit einer Referenz referenziert wird.
  const std::string& s = getString();
  std::cout << s << std::endl;

  // Fall 3:
  std::vector<int> vNum = {};
  vNum.reserve(2);

  vNum.push_back(1);
  vNum.push_back(2);

  int& vNumRef = vNum[1];
  std::cout << vNumRef << std::endl;

  vNum.push_back(3);

  // ASan: SUMMARY: AddressSanitizer: heap-use-after-free main.cpp:56 in main
  // vNumRef zeigt in den alten Heap-Buffer von vNum. push_back(3) sprengt die
  // reservierte Kapazitaet (2), vector alloziert einen neuen Buffer, kopiert/
  // moved die Elemente rueber und gibt den alten frei. vNumRef zeigt danach
  // auf freigegebenen Speicher.
  // should be invalid
  std::cout << vNumRef << std::endl;

  // Fall 4:
  using SomeArr = std::array<int, 8>;
  std::map<std::string, SomeArr> myMap;
  myMap.insert({"test", {1, 2, 3, 4, 5}});
  SomeArr& arrAtTest = myMap.at("test");

  myMap.erase("test");

  // ASan: SUMMARY: AddressSanitizer: heap-use-after-free main.cpp:71 in main
  // arrAtTest referenziert den mapped_type-Wert des Knotens "test". erase()
  // zerstoert diesen Knoten und gibt seinen Speicher frei. arrAtTest zeigt
  // danach auf freigegebenen Speicher.
  // should fail
  std::cout << arrAtTest[2] << std::endl;

  std::cout << "Hello World" << std::endl;
  return 0;
}