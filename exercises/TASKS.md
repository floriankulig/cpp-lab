# C++17 Grundlagen — Aufgabenplan

Ziel dieser Reihe: nicht Syntax, sondern **Mechanik**. Nach jeder Aufgabe sollst
du erklären können, was der Compiler generiert hat, was wann im Speicher passiert
und warum die naive Variante kaputt ist.

Kein Code von mir. Ich stelle Spezifikation und Akzeptanzkriterien, du baust.
Danach reviewen wir gemeinsam.

## So arbeiten wir

1. Du liest die Aufgabe und **schreibst zuerst deine Erwartung auf** (als
   Kommentar im Code) — bei Aufgaben mit Ausgabe: die vorhergesagte Ausgabe.
2. Du implementierst.
3. Du prüfst gegen die Akzeptanzkriterien.
4. Du sagst mir „fertig mit 03" — ich reviewe: Korrektheit/UB zuerst, dann
   Idiomatik, dann Stil.
5. Steckenbleiben ist erwünscht. Frag mit konkreter Frage („warum invalidiert
   das den Iterator?"), nicht mit „geht nicht".

Die Denkfragen am Ende jeder Aufgabe sind Teil der Aufgabe. Wenn du sie nicht
beantworten kannst, ist die Aufgabe nicht fertig — dann frag mich.

## Setup

Ein Verzeichnis pro Aufgabe: `exercises/01-tracer/main.cpp`.

Bauen und laufen lassen:

```sh
clang++ -std=c++17 -Wall -Wextra -Wpedantic -g \
    -fsanitize=address,undefined main.cpp -o /tmp/x && /tmp/x
```

Warnungen sind Fehler — es soll nichts übrig bleiben.

Leak-Check (ASan kann das auf macOS/arm64 **nicht**):

```sh
clang++ -std=c++17 -g main.cpp -o /tmp/x && leaks --atExit -- /tmp/x
```

Optimierte Messungen (nur für Aufgabe 08): `-O2` statt `-g`, ohne Sanitizer.

## Fortschritt

**Block A — Wertsemantik & Lebensdauer** (das Fundament, nicht überspringen)

- [x] 01 — Tracer: wann wird kopiert, verschoben, zerstört
- [x] 02 — Dangling: vier Wege, Speicher zu verlieren
- [x] 11 — Initialisierung: die Fallen _(kann direkt nach 01 kommen)_
- [ ] 12 — Copy Elision in C++17

**Block B — Ownership selbst gebaut**

- [x] 03 — `Vector<T>` v1: Rule of Three
- [ ] 04 — `Vector<T>` v2: Rule of Five, Allokation ≠ Konstruktion
- [ ] 05 — `UniquePtr<T>`: move-only
- [ ] 06 — `SharedPtr<T>` + Zyklus mit `weak_ptr` brechen

**Block C — Container & Iteratoren im Alltag**

- [ ] 07 — Iterator-Invalidierung
- [ ] 09 — `std::map`: `[]` vs `at` vs `find` vs `emplace` vs `try_emplace`
- [ ] 10 — Eigene Typen in `set`/`unordered_map`
- [ ] 08 — Containerwahl & Cache-Lokalität _(optional, aber lehrreich)_

**Block D — Robustheit**

- [ ] 13 — RAII um eine C-Ressource
- [ ] 14 — Exception-Sicherheit im eigenen Vector

Reihenfolge-Empfehlung: 01 → 02 → 11 → 12 → 03 → 05 → 07 → 09 → 04 → 06 → 10 →
13 → 14 → 08.

---

# Block A — Wertsemantik & Lebensdauer

## 01 — Tracer

**Ziel:** Du siehst zum ersten Mal _tatsächlich_, wann C++ kopiert, verschiebt
und zerstört — statt es zu vermuten.

**Aufgabe:**

Baue eine Klasse `Tracer`, die jedem Objekt eine fortlaufende ID gibt und in
**jedem** der sechs Spezialmember eine Zeile ausgibt, z. B.
`Tracer#3  copy-ctor from #1`. Die sechs: Default-Ctor, Copy-Ctor, Move-Ctor,
Copy-Assignment, Move-Assignment, Destruktor.

Dann eine `main`, die folgende Szenarien nacheinander durchspielt — mit
Trennzeilen dazwischen und **vorher notierter Prognose** als Kommentar:

1. Übergabe an eine Funktion `by value` vs. `by const&`
2. Rückgabe eines lokalen `Tracer` aus einer Funktion
3. Rückgabe eines temporären `Tracer{}` aus einer Funktion
4. `std::vector<Tracer> v; v.push_back(t);` mit `t` als lvalue
5. dasselbe mit `v.push_back(std::move(t))`
6. `v.emplace_back()` — direkt konstruiert
7. fünf `push_back` in Folge **ohne** vorheriges `reserve`
8. dieselben fünf **mit** `v.reserve(5)` davor
9. `std::move` auf ein `const Tracer` — dann Kopie oder Move?
10. `std::swap(a, b)` auf zwei Tracer

**Akzeptanzkriterien:**

- Jedes Szenario ist im Output eindeutig abgegrenzt.
- Für jede Abweichung Prognose ↔ Realität steht ein Kommentar im Code mit deiner
  Erklärung.
- Szenario 7 zeigt sichtbar die Reallocations (Moves + Destruktoren der alten
  Elemente), Szenario 8 zeigt keine.
- Keine Warnungen, Sanitizer sauber.

**Denkfragen:**

- Warum verschiebt `vector` beim Umkopieren nur dann, wenn dein Move-Ctor
  `noexcept` ist? Teste es: nimm das `noexcept` weg und vergleiche Szenario 7.
- Was liefert `std::move(constObjekt)` — und warum ist das Ergebnis eine Kopie?
- Warum sind es in Szenario 3 weniger Aufrufe, als du gelernt hast? (→ Aufgabe 12)

Diesen `Tracer` brauchst du in 04, 09 und 12 wieder. Leg ihn so ab, dass du ihn
kopieren kannst.

---

## 02 — Dangling: vier Wege, Speicher zu verlieren

**Ziel:** Lebensdauer als eigenständiges Konzept begreifen — unabhängig von
Scope-Syntax.

**Aufgabe:** Vier eigenständige kleine Programme (oder vier Funktionen mit
Schaltern), die je **einen** klassischen Lebensdauerfehler erzeugen. Jeweils
zuerst die kaputte Version reproduzieren, dann korrigieren:

1. Eine Funktion gibt eine Referenz auf eine lokale Variable zurück.
2. Ein `std::string_view` zeigt auf ein Temporary
   (`std::string_view sv = getString();`) und wird danach gelesen.
3. Ein Zeiger/eine Referenz auf ein `vector`-Element wird nach einem `push_back`
   benutzt, das reallociert.
4. Eine Referenz auf ein `std::map`-Element wird nach dem `erase` dieses
   Elements benutzt.

Zusatz ohne Bug: zeige, dass `const std::string& s = getString();` **legal** ist
(Lifetime Extension) — und erkläre im Kommentar, warum dieselbe Regel bei
`string_view` nicht greift.

**Akzeptanzkriterien:**

- Für Fall 1, 3, 4 produziert AddressSanitizer einen Report
  (`stack-use-after-return` braucht ggf. `ASAN_OPTIONS=detect_stack_use_after_return=1`);
  kopiere die erste Zeile des Reports als Kommentar in den Code.
- Fall 2 zeigt sichtbar Müll oder crasht.
- Jede Fixversion läuft sanitizer-sauber.
- Pro Fall ein Kommentar: **welches Objekt** war wann tot, und **wer** hielt noch
  eine Referenz darauf.

**Denkfragen:**

- Warum warnt der Compiler bei 1, aber nicht bei 3?
- `string_view` und `span` sind „Borrow"-Typen. Welche Regel gilt für sie als
  Rückgabetyp einer Funktion?

---

## 11 — Initialisierung: die Fallen

**Ziel:** Die Initialisierungsregeln sind der häufigste Ort für stille Bugs.

**Aufgabe:** Pro Punkt ein 3–8-Zeiler mit Kommentar, was passiert und warum:

1. `std::vector<int> v(3, 5);` vs. `std::vector<int> v{3, 5};` — Größe? Inhalt?
2. Most Vexing Parse: `Widget w();` vs. `Widget w{};` — was ist `w` im ersten Fall?
3. Member-Initialisierungsreihenfolge: Klasse mit zwei Membern, im
   Initialisierer-Listing **absichtlich** in falscher Reihenfolge, wobei der
   zweite den ersten benutzt. Compiler mit `-Wall` beobachten, Ergebnis erklären.
4. `struct S { int a; };` — `S s1;` vs. `S s2{};` vs. `S s3 = S();`:
   Welcher Member ist uninitialisiert? Mit UBSan/`-Wuninitialized` nachweisen.
5. `= default` im Klassenrumpf vs. `S() {}` — Unterschied bei den Membern.
6. Default Member Initializer (`int a = 0;`) kombiniert mit einem Ctor, der `a`
   ebenfalls setzt — wer gewinnt?
7. `explicit` Ctor: baue einen Ctor mit einem Parameter, zeige die implizite
   Konvertierung, dann `explicit` davor und zeige den Compilerfehler.
8. Delegating Constructor: drei Ctors, einer macht die Arbeit.
9. Ein `const`-Member oder Referenz-Member: was passiert mit
   Copy-/Move-Assignment? (Compilerfehler lesen und erklären.)

**Akzeptanzkriterien:**

- Alles kompiliert bzw. die geforderten Compilerfehler sind als Kommentar
  festgehalten (Zeile + Kernaussage der Fehlermeldung).
- Zu Punkt 4 kannst du sagen, wann „uninitialisiert" und wann „zero-initialized"
  gilt.

**Denkfragen:**

- Wann bevorzugst du `{}` und wann `()`? Formuliere eine Regel, die du
  verteidigen kannst.
- Warum ist `std::vector<int> v{3, 5};` so gefährlich in generischem Code?

---

## 12 — Copy Elision in C++17

**Ziel:** Verstehen, was C++17 garantiert — und warum `return std::move(x)` ein
Anti-Pattern ist.

**Aufgabe:** Mit dem `Tracer` aus 01, drei Funktionen:

```
a) Tracer f() { return Tracer{}; }        // prvalue
b) Tracer g() { Tracer t; return t; }     // NRVO-Kandidat
c) Tracer h() { Tracer t; return std::move(t); }
```

Jeweils `auto x = f();` etc. aufrufen und die Ausgabe vergleichen. Danach alles
nochmal mit `-fno-elide-constructors` bauen und die Ausgaben gegenüberstellen.

**Akzeptanzkriterien:**

- Tabelle im Kommentar: Funktion × Flag → welche Spezialmember liefen.
- Du kannst begründen, welcher der drei Fälle durch C++17 **garantiert**
  elidiert wird und welcher nur _optional_ optimiert wird.
- Du kannst erklären, warum (c) schlechter ist als (b).

---

# Block B — Ownership selbst gebaut

## 03 — `Vector<T>` v1: Rule of Three

**Ziel:** Besitz, Freigabe und Selbstzuweisung — die kleinste vollständige
ressourcenhaltende Klasse.

**Aufgabe:** Ein `template <typename T> class Vector` mit `new[]`/`delete[]` als
Speicherstrategie. Geforderte API:

- `Vector()`, `Vector(std::size_t count, const T& value)`
- Copy-Ctor, Copy-Assignment, Destruktor
- `push_back(const T&)`
- `size()`, `capacity()`, `empty()`
- `operator[]` in const- und non-const-Variante
- `begin()`/`end()` (rohe `T*` reichen), const-Varianten
- `at()` mit `std::out_of_range`

**Akzeptanzkriterien:**

- `-Wall -Wextra -Wpedantic` ohne eine einzige Warnung.
- ASan sauber; `leaks --atExit` meldet 0 Leaks.
- `v = v;` (Selbstzuweisung) funktioniert korrekt und leakt nicht.
- Range-based `for` läuft über deinen Vector.
- Wachstum ist amortisiert konstant (Kapazität verdoppelt sich); belege es mit
  einer Ausgabe von `capacity()` nach jedem `push_back` bis 10.
- `const Vector<int> cv{...}; cv[0];` kompiliert; `cv[0] = 5;` nicht.

**Denkfragen:**

- Warum braucht es `operator[]` zweimal, und was unterscheidet die Rückgabetypen?
- Was genau geht schief, wenn du in `operator=` zuerst `delete[]` machst und die
  Selbstzuweisung nicht abfängst?
- Was ist das Copy-and-Swap-Idiom und welches Problem löst es hier?
- Warum ist `new T[n]` für einen echten `std::vector` untauglich? (→ Aufgabe 04)

---

## 04 — `Vector<T>` v2: Rule of Five, Allokation ≠ Konstruktion

**Ziel:** Der Kern von `std::vector`: Speicher besorgen und Objekte darin
konstruieren sind **zwei verschiedene Dinge**.

**Aufgabe:** Baue 03 um auf rohen Speicher:

- Allokation mit `::operator new(bytes)` / Freigabe mit `::operator delete`
- Konstruktion mit **Placement new**, Zerstörung mit explizitem
  Destruktoraufruf (`p->~T()`)
- Move-Ctor und Move-Assignment (beide `noexcept`), Copy-Varianten bleiben
- `reserve(n)`, `pop_back()`, `clear()`
- `emplace_back(Args&&...)` mit Perfect Forwarding
- Wachstum nutzt `std::move_if_noexcept`

**Akzeptanzkriterien:**

- `Vector<Tracer> v; v.reserve(4);` konstruiert **null** Tracer (Nachweis über
  die Tracer-Ausgabe). Das ist der zentrale Unterschied zu 03.
- Fünf `push_back` ab Kapazität 4 zeigen genau eine Reallocation, und dabei
  **Moves**, keine Copies.
- `clear()` ruft alle Destruktoren, `capacity()` bleibt unverändert.
- Move-Ctor hinterlässt die Quelle in gültigem, leerem Zustand (`size()==0`,
  danach ist `push_back` auf der Quelle noch erlaubt).
- ASan sauber, `leaks` 0.

**Denkfragen:**

- Warum `move_if_noexcept` statt `std::move`? Was passiert beim Realloc, wenn
  ein Move-Ctor mitten in der Schleife wirft?
- Warum müssen Move-Operationen `noexcept` sein, damit `std::vector` sie nutzt?
- `emplace_back(args...)` vs. `push_back(T(args...))` — wie viele Objekte
  entstehen jeweils? Miss es mit `Tracer`.

---

## 05 — `UniquePtr<T>`

**Ziel:** Move-only-Typen, exklusiver Besitz, und warum `explicit` hier zählt.

**Aufgabe:** Geforderte API:

- `explicit UniquePtr(T* p)`, `UniquePtr()` (nullptr), Destruktor
- Copy-Ctor und Copy-Assignment **gelöscht**
- Move-Ctor und Move-Assignment (`noexcept`)
- `get()`, `release()`, `reset(T* p = nullptr)`, `swap()`
- `operator*`, `operator->`, `explicit operator bool`
- freie Funktion `template <typename T, typename... Args> UniquePtr<T> MakeUnique(Args&&...)`

**Akzeptanzkriterien:**

- `static_assert(!std::is_copy_constructible_v<UniquePtr<int>>)` und
  `static_assert(std::is_move_constructible_v<UniquePtr<int>>)` compilen.
- `static_assert(std::is_nothrow_move_constructible_v<UniquePtr<int>>)` compiliert.
- Selbstzuweisung `p = std::move(p)` crasht nicht und leakt nicht.
- Mit `Tracer` als `T`: Destruktor läuft **genau einmal**, zum richtigen
  Zeitpunkt.
- `UniquePtr<Tracer>` in einem `std::vector` funktioniert (push_back mit
  `std::move`), Realloc verschiebt.
- ASan sauber, `leaks` 0.

**Denkfragen:**

- Warum muss der Ctor `explicit` sein? Konstruiere den Bug, der ohne entsteht.
- In `reset(p)`: in welcher Reihenfolge musst du altes Löschen und neues
  Zuweisen ausführen — und warum ist die intuitive Reihenfolge falsch?
- Warum ist `operator bool` `explicit`, `operator->` aber nicht?

---

## 06 — `SharedPtr<T>` + Zyklen

**Ziel:** Referenzzählung verstehen, inklusive der Stelle, an der sie versagt.

**Teil A (zuerst, mit der Standardbibliothek):**
Baue `struct Node { std::shared_ptr<Node> other; Tracer t; };` und erzeuge einen
Zyklus zweier Nodes. Weise nach, dass die Destruktoren **nicht** laufen (fehlende
Tracer-Ausgabe) und `leaks` einen Leak meldet. Dann fixe es mit `std::weak_ptr`
und weise nach, dass beide Destruktoren laufen.

**Teil B (dann selbst bauen):**
`SharedPtr<T>` mit separatem Control Block (strong count), Copy/Move, `use_count()`,
`reset()`, `MakeShared`. Danach `WeakPtr<T>` mit weak count und `lock()`.

**Akzeptanzkriterien:**

- Teil A: Leak nachgewiesen **und** behoben, jeweils mit Beleg im Kommentar.
- Teil B: `use_count()` stimmt in einer Sequenz aus Kopien in verschachtelten
  Scopes (schreib die erwarteten Werte als `assert`).
- Das verwaltete Objekt wird genau einmal zerstört, der Control Block genau
  einmal freigegeben.
- `WeakPtr::lock()` liefert einen leeren `SharedPtr`, nachdem der letzte
  `SharedPtr` weg ist — ohne Zugriff auf freigegebenen Speicher.
- ASan sauber, `leaks` 0.

**Denkfragen:**

- Warum sind es zwei Zähler und nicht einer?
- Warum spart `make_shared` eine Allokation — und welchen Nachteil hat das im
  Zusammenspiel mit `weak_ptr`?
- Warum muss der Zähler atomar sein, `shared_ptr` selbst aber trotzdem nicht
  thread-safe ist?

---

# Block C — Container & Iteratoren im Alltag

## 07 — Iterator-Invalidierung

**Ziel:** Die Regel, an der in echten Codebases die meisten Speicherbugs hängen.

**Aufgabe:**

Teil 1 — Erarbeite dir experimentell eine Tabelle (als Markdown-Kommentar oder
`NOTES.md` im Aufgabenordner): für `vector`, `deque`, `list`, `map`,
`unordered_map` jeweils, was `insert`/`push_back` und `erase` invalidieren —
**Iteratoren** und **Referenzen/Zeiger** getrennt betrachtet.

Teil 2 — Alle geraden Zahlen aus einem `std::vector<int>` entfernen, in drei
Varianten:

1. die naive Schleife mit `erase` im Body **ohne** den Rückgabewert zu nutzen
   (kaputt — zeig, wie sie falsche Ergebnisse liefert oder crasht)
2. korrekt mit `it = v.erase(it)`
3. mit dem Erase-Remove-Idiom (`std::remove_if` + `erase`)

Teil 3 — Dasselbe für `std::map`: Einträge nach Bedingung löschen mit
`it = m.erase(it)`. Zeige zusätzlich, dass eine Referenz auf ein _anderes_
map-Element das `erase` überlebt (bei `vector` nicht).

**Akzeptanzkriterien:**

- Variante 1 liefert nachweisbar ein falsches Ergebnis oder einen ASan-Report;
  beides festgehalten.
- Varianten 2 und 3 liefern identische, korrekte Ergebnisse (`assert`).
- Die Tabelle ist vollständig und du kannst zu jeder Zelle sagen **warum**
  (Speicherlayout des Containers).

**Denkfragen:**

- Warum überleben `std::map`-Referenzen fast alles, `std::vector`-Referenzen fast
  nichts?
- Was macht `std::remove_if` eigentlich — und warum braucht es das zweite
  `erase`?
- `unordered_map`: was passiert bei einem Rehash mit Iteratoren, was mit
  Referenzen?

---

## 09 — `std::map`: `[]` vs. `at` vs. `find` vs. `insert` vs. `emplace` vs. `try_emplace`

**Ziel:** Die tägliche Map-Falle: unbeabsichtigtes Einfügen und doppelte
Lookups.

**Aufgabe:** Ein Wortzähler über einen fest im Code stehenden Text
(`std::map<std::string, int>`), implementiert in mehreren Varianten. Zusätzlich
eine `std::map<std::string, Tracer>`, um Konstruktionen zu **zählen**.

Zu untersuchen:

- `m[key]++` — wie viele Lookups? Was passiert bei unbekanntem Key?
- `m.at(key)` bei unbekanntem Key
- `if (auto it = m.find(k); it != m.end())` (C++17 If-Init)
- `m.insert({k, v})` vs. `m.emplace(k, v)` vs. `m.try_emplace(k, args...)` —
  wie viele Konstruktionen des Wertes bei **bereits vorhandenem** Key?
- `insert_or_assign`
- Rückgabewert von `insert`/`emplace` (`pair<iterator, bool>`) mit Structured
  Bindings auswerten

**Akzeptanzkriterien:**

- Tabelle: Variante × (Lookups, Wert-Konstruktionen bei vorhandenem Key,
  Wert-Konstruktionen bei neuem Key), belegt durch die Tracer-Ausgabe.
- Du kannst erklären, warum `operator[]` auf einer `const std::map` nicht
  existiert.
- Mindestens eine Variante nutzt C++17-If-Init und Structured Bindings.

**Denkfragen:**

- Wann ist `emplace` schlechter als `try_emplace`?
- Warum ist `m[key]` für einen reinen Lesezugriff ein Bug, kein Stilproblem?

---

## 10 — Eigene Typen in `set` und `unordered_map`

**Ziel:** Was ein Container von deinem Typ verlangt — und was passiert, wenn du
es verletzt.

**Aufgabe:**

1. `struct Point { int x, y; };` in ein `std::set<Point>` legen. Ordnung über
   `operator<` **oder** ein Comparator-Funktionsobjekt als Template-Argument —
   implementiere beide Wege.
2. Denselben `Point` in ein `std::unordered_map<Point, int>` legen:
   `operator==` plus Spezialisierung von `std::hash<Point>`. Kombiniere die
   Hashes sinnvoll (nicht `x + y`).
3. Baue **absichtlich** einen kaputten Comparator (`<=` statt `<`). Schreibe eine
   Prüffunktion, die über alle Paare/Tripel eines kleinen Testsets die Axiome
   einer _strict weak ordering_ prüft (Irreflexivität, Asymmetrie,
   Transitivität, Transitivität der Äquivalenz) und den Bruch meldet.

**Akzeptanzkriterien:**

- Beide Container funktionieren; Duplikate werden korrekt erkannt.
- Die Prüffunktion meldet beim kaputten Comparator einen konkreten Verstoß und
  beim korrekten keinen.
- Ein Hash-Test: zwei gleiche Points haben denselben Hash (`assert`).

**Denkfragen:**

- Warum reicht `operator==` für `unordered_map` nicht und `std::hash` allein
  auch nicht?
- Was ist der formale Vertrag zwischen `hash` und `==`? Was passiert, wenn du ihn
  brichst?
- Warum ist ein Comparator mit `<=` **undefiniertes Verhalten** und nicht nur
  „falsche Sortierung"?

---

## 08 — Containerwahl & Cache-Lokalität _(optional)_

**Ziel:** Warum O-Notation die Praxis nicht entscheidet.

**Aufgabe:** Mit `std::chrono::steady_clock` messen, jeweils 100 000 Elemente:

- Aufbau: `vector` (mit und ohne `reserve`) vs. `list` vs. `deque`
- Sequentielles Aufsummieren aller Elemente: `vector` vs. `list`
- Einfügen in die Mitte: `vector` vs. `list` (inkl. der Kosten, die Mitte zu
  _finden_)
- 1 000 000 Lookups: `std::map` vs. `std::unordered_map`

**Akzeptanzkriterien:**

- Gemessen wird mit `-O2` **ohne** Sanitizer; jede Messung mindestens 3-mal,
  Median berichtet.
- Ergebnisse werden gegen Wegoptimieren geschützt (Summe verwenden/ausgeben).
- Ergebnistabelle plus eine Erklärung: warum ist `list` beim Summieren um ein
  Vielfaches langsamer, obwohl beides O(n) ist?

**Denkfragen:**

- Wann gewinnt `list` tatsächlich?
- Warum ist `deque` beim Aufbau oft gut, beim Iterieren aber langsamer als
  `vector`?
- Was hat das mit Cache-Lines und Prefetching zu tun?

---

# Block D — Robustheit

## 13 — RAII um eine C-Ressource

**Ziel:** Das Muster, das du in echtem Code am häufigsten selbst schreiben wirst.

**Aufgabe:** `class FileHandle` um `std::FILE*` (`std::fopen`/`std::fclose`):

- Ctor öffnet, wirft `std::runtime_error` bei Fehler
- Destruktor schließt, `noexcept`, robust gegen „schon released"
- Move-only: Move-Ctor/Assignment `noexcept`, Copy gelöscht
- `write(std::string_view)`, `explicit operator bool`, `get()`
- Move hinterlässt die Quelle in gültigem, leerem Zustand

**Akzeptanzkriterien:**

- Doppeltes Schließen ist strukturell unmöglich (Test: Move und dann beide
  Objekte zerstören lassen).
- Wird zwischen Öffnen und Ende eine Exception geworfen, ist die Datei trotzdem
  geschlossen — weise es nach (z. B. über einen Log im Destruktor).
- `std::vector<FileHandle>` mit mehreren Elementen und `reserve`-losem Wachstum
  funktioniert.
- `static_assert(std::is_nothrow_move_constructible_v<FileHandle>)`.
- ASan sauber, `leaks` 0.

**Denkfragen:**

- Warum darf ein Destruktor nicht werfen? Was passiert konkret, wenn er es
  während Stack Unwinding tut?
- Was macht `std::vector` beim Realloc, wenn dein Move-Ctor **nicht** `noexcept`
  ist — und warum ist das für einen Ressourcen-Typ ein Problem?
- Wie sähe die Variante mit `std::unique_ptr` und Custom Deleter aus? (Nur
  beschreiben, nicht bauen.)

---

## 14 — Exception-Sicherheit im eigenen Vector

**Ziel:** Die drei Garantien (keine/basis/stark) an einem Fall, den du selbst
kaputt gemacht hast.

**Aufgabe:** Ein Typ `Throwy`, dessen Copy-Ctor beim n-ten Aufruf
`std::runtime_error` wirft (n konfigurierbar). Damit:

1. Zeige, dass dein `Vector` aus 04 beim Realloc mit `Throwy` in einen kaputten
   Zustand gerät oder leakt (Beleg: `leaks`, oder inkonsistente `size()`).
2. Implementiere `push_back` mit **starker Garantie**: neuen Puffer allozieren,
   dort konstruieren, erst bei vollständigem Erfolg tauschen und den alten
   freigeben.
3. Räume auch den halbfertigen neuen Puffer korrekt auf, wenn mittendrin
   geworfen wird.

**Akzeptanzkriterien:**

- Nach der geworfenen Exception sind `size()`, `capacity()` und alle Elemente
  **unverändert** (mit `assert` geprüft).
- Keine Leaks (`leaks --atExit`), auch nicht im Wurf-Pfad.
- Du kannst pro Methode deines Vectors sagen, welche Garantie sie gibt.

**Denkfragen:**

- Warum kann `push_back` mit `noexcept`-Move-Ctor die starke Garantie geben,
  mit werfendem Move-Ctor aber nicht? (Das ist der eigentliche Grund für
  `move_if_noexcept`.)
- Welche Garantie gibt `clear()`? Welche `reserve()`?
- Warum ist `swap` als `noexcept` die Grundlage fast aller starken Garantien?
