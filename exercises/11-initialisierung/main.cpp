// Main Template
#include <iostream>
#include <vector>

struct Widget {};

int main() {
  // Fall 1:
  //  Ich habe nachgeguckt und die Dokumentation durchgelesen. Das erste
  //  Assignment erstellt einen Vektor der Größe 3 mit allen Elementen darin als
  //  5 initialisiert.
  std::vector<int> v(3, 5);
  // Und hier erstellen wir einen Vektor der Größe 2 mit dem ersten Element 3
  // und dem zweiten Element 5.
  std::vector<int> v2{3, 5};
  std::cout << v[2] << std::endl;

  // Fall 2:
  // Hier wird ein Objekt Widget erstellt.
  Widget w{};
  // Hier weiß der Compiler nicht, ob das jetzt eine Funktionsdeklaration ist
  // oder ob hier ein Objekt vom Typ Widget erstellt werden soll. Das ist der
  // Most Vexing Parse.
  Widget w2();

  // Fall 3:
  struct SomeClass {
    int a_, b_;

    // Hier ist A noch nicht initialisiert. Deswegen wird B auf jeden Fall
    // das falsche Ergebnis zugewiesen bekommen.
    SomeClass(int first, int second) : b_{a_ + second}, a_{first} {}
  };
  SomeClass sc{1, 5};
  std::cout << sc.a_ << std::endl;
  std::cout << sc.b_ << std::endl;

  // Fall 4:
  struct S {
    int a;
  };
  // Das hier dürfte nicht initialisiert sein, glaube ich.
  S s1;
  // Das hier sollte passen. zero-initialized.
  S s2{};
  // Hier weiß ich es nicht, aber ich glaube, das wird auch nochmal
  // initialisiert. Also dann ist der private Integer A einfach 0.
  S s3 = S();

  // Fall 5:
  // Hier schreibe ich einfach so, weil ich keine Lust habe, das Struct neu zu
  // schreiben.
  // Aufgabe: = default im Klassenrumpf vs. S() {} — Unterschied bei den
  // Membern. Ich habe keine Ahnung, was hier passiert. Ich würde aber sagen,
  // beim zweiten Fall, also S() {} wird A erst uninitialized sein und dann
  // default mäßig Zero initialized und bei default wird das einfach direkt zero
  // initialized

  // Fall 6:
  // Auch hier habe ich keine Lust, das nochmal neu zu implementieren. Ich sage,
  // dass der Constructor gewinnt.

  // Fall 7:
  // Konvertiert das dann zu double?
  class Conv {
    float a_;
    Conv(int a) {}
  };

  // Fall 8: Keine Ahnung, was du hier von mir willst.

  // Fall 9: Das kann ich auch nicht selbst implementieren. Keine Ahnung wie man
  // das baut und testet.

  std::cout << "Hello World" << std::endl;
  return 0;
}