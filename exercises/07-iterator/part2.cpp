// Teil 2 -- alle geraden Zahlen aus einem vector entfernen, drei Varianten.
//
// Aufruf:  /tmp/x        -> Variante 1 (falsches Ergebnis), 2 und 3 mit assert
//          /tmp/x crash  -> Variante 1 mit gerader Zahl am Ende (ASan-Report)
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

bool isEven(int x) { return x % 2 == 0; }

void print(const char* label, const std::vector<int>& v) {
  std::cout << label << ':';
  for (int x : v)
    std::cout << ' ' << x;
  std::cout << '\n';
}

// Variante 1 -- KAPUTT.
// erase(it) schiebt alle Elemente hinter `it` eine Position nach vorne.
// Formal ist `it` danach ungueltig; praktisch zeigt der Zeiger darin jetzt
// auf das Element, das nachgerutscht ist. Das `++it` im Schleifenkopf
// springt genau ueber dieses Element hinweg -> es wird nie geprueft.
// Ist das letzte Element gerade, steht `it` nach dem erase genau auf dem
// neuen end(); `++it` springt darueber hinaus, `it != end()` bleibt wahr,
// und `*it` liest hinter dem Puffer.
void eraseNaive(std::vector<int>& v) {
  for (auto it = v.begin(); it != v.end(); ++it) {
    if (isEven(*it)) {
      v.erase(it); // Rueckgabewert ignoriert
    }
  }
}

// Variante 2 -- korrekt.
// erase gibt einen gueltigen Iterator auf das nachgerutschte Element zurueck.
// Deshalb nur weiterschalten, wenn nichts geloescht wurde.
void eraseLoop(std::vector<int>& v) {
  for (auto it = v.begin(); it != v.end();) {
    if (isEven(*it)) {
      it = v.erase(it);
    } else {
      // ++it;
      it++;
    }
  }
}

// Variante 3 -- Erase-Remove-Idiom.
// remove_if loescht nichts: es moved alle Elemente, die bleiben sollen, nach
// vorne und gibt einen Iterator auf das neue logische Ende zurueck. size()
// ist danach unveraendert, hinten liegt Restmuell. Erst das
// erase(newEnd, end()) verkleinert den vector wirklich.
// O(n) statt O(n^2) wie bei Variante 2 (jedes erase dort schiebt den Rest).
void eraseRemove(std::vector<int>& v) {
  v.erase(std::remove_if(v.begin(), v.end(), isEven), v.end());
}

int main(int argc, char** argv) {
  if (argc > 1 && std::strcmp(argv[1], "crash") == 0) {
    // Letztes Element gerade; aus Initializer-Liste -> capacity == size,
    // also liegt direkt hinter end() kein Speicher mehr, den ASan erlaubt.
    std::vector<int> v{1, 2};
    eraseNaive(v);
    // ASan: heap-buffer-overflow, READ of size 4 in eraseNaive (Zeile 29,
    // `isEven(*it)`), "0 bytes after 8-byte region" -> direkt hinter dem
    // Puffer.
    return 0;
  }

  // Zwei gerade Zahlen hintereinander (2,4 und 6,8), letzte Zahl ungerade.
  const std::vector<int> input{1, 2, 4, 5, 6, 8, 9};

  std::vector<int> v1 = input;
  eraseNaive(v1);
  print("Variante 1", v1);
  // Ausgabe: "Variante 1: 1 4 5 8 9" -- 4 und 8 sind uebrig, weil sie nach dem
  // erase ihres Vorgaengers (2 bzw. 6) nachgerutscht und von ++it
  // uebersprungen wurden.

  std::vector<int> v2 = input;
  eraseLoop(v2);
  print("Variante 2", v2);

  std::vector<int> v3 = input;
  eraseRemove(v3);
  print("Variante 3", v3);

  assert(v2 == v3);
  assert(v2 == (std::vector<int>{1, 5, 9}));
  assert(std::none_of(v2.begin(), v2.end(), isEven));
  std::cout << "Varianten 2 und 3: ok\n";
}
