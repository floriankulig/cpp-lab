// Teil 1 -- Messharness: welche Elemente wechseln bei einer Operation ihre
// Adresse? Gleiche Adresse => Referenz/Zeiger bleibt gueltig.
// Iteratoren lassen sich nicht ohne UB messen -> Spalte selbst ableiten.
#include <algorithm>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

// Adresse des Elements mit Wert (Sequenz) bzw. Key (Map), nullptr falls weg.
template <class C> const void* addr(const C& c, int v) {
  auto it = std::find(c.begin(), c.end(), v);
  return it == c.end() ? nullptr : &*it;
}
template <class M> const void* mapAddr(const M& m, int k) {
  auto it = m.find(k);
  return it == m.end() ? nullptr : &it->second;
}
const void* addr(const std::map<int, int>& m, int k) { return mapAddr(m, k); }
const void* addr(const std::unordered_map<int, int>& m, int k) {
  return mapAddr(m, k);
}

template <class C> std::string meta(const C&) { return ""; }
std::string meta(const std::vector<int>& v) {
  return "capacity=" + std::to_string(v.capacity());
}
std::string meta(const std::unordered_map<int, int>& m) {
  return "buckets=" + std::to_string(m.bucket_count());
}

// Beobachtet Element 2 (vor der Mitte) und 7 (dahinter).
template <class C, class Op> void probe(const std::string& label, C c, Op op) {
  const int watch[] = {2, 7};
  const void* before[] = {addr(c, watch[0]), addr(c, watch[1])};
  std::string metaBefore = meta(c);
  op(c);
  std::cout << label;
  if (!metaBefore.empty()) std::cout << "  [" << metaBefore << " -> " << meta(c) << "]";
  std::cout << '\n';
  for (int i = 0; i < 2; ++i)
    std::cout << "    Element " << watch[i] << ": "
              << (addr(c, watch[i]) == before[i] ? "gleiche Adresse" : "VERSCHOBEN")
              << '\n';
}

template <class C> C seq() {
  C c(10);
  std::iota(c.begin(), c.end(), 0);  // 0..9
  return c;
}
template <class M> M mapOf() {
  M m;
  for (int i = 0; i < 10; ++i) m[i] = i;
  return m;
}

using V = std::vector<int>;
using D = std::deque<int>;
using L = std::list<int>;
using M = std::map<int, int>;
using U = std::unordered_map<int, int>;

// Per Rueckgabe statt Kopie: eine Kopie wuerde reserve() nicht mitnehmen.
V reservedVec() {
  V v = seq<V>();
  v.reserve(20);
  return v;
}
U reservedMap() {
  U m = mapOf<U>();
  m.reserve(100);
  return m;
}

int main() {
  std::cout << "--- vector ---\n";
  probe("push_back, capacity reicht", reservedVec(), [](V& v) { v.push_back(100); });
  probe("push_back, capacity voll", seq<V>(), [](V& v) { v.push_back(100); });
  probe("insert Mitte, capacity reicht", reservedVec(), [](V& v) { v.insert(v.begin() + 5, 100); });
  probe("erase Mitte (5)", seq<V>(), [](V& v) { v.erase(v.begin() + 5); });

  std::cout << "--- deque ---\n";
  probe("push_back", seq<D>(), [](D& d) { d.push_back(100); });
  probe("push_front", seq<D>(), [](D& d) { d.push_front(100); });
  probe("insert Mitte", seq<D>(), [](D& d) { d.insert(d.begin() + 5, 100); });
  probe("erase Anfang (0)", seq<D>(), [](D& d) { d.erase(d.begin()); });
  probe("erase Mitte (5)", seq<D>(), [](D& d) { d.erase(d.begin() + 5); });

  std::cout << "--- list ---\n";
  probe("push_back", seq<L>(), [](L& l) { l.push_back(100); });
  probe("insert Mitte", seq<L>(), [](L& l) { l.insert(std::next(l.begin(), 5), 100); });
  probe("erase Mitte (5)", seq<L>(), [](L& l) { l.erase(std::next(l.begin(), 5)); });

  std::cout << "--- map ---\n";
  probe("insert", mapOf<M>(), [](M& m) { m.insert({100, 100}); });
  probe("erase (5)", mapOf<M>(), [](M& m) { m.erase(5); });

  std::cout << "--- unordered_map ---\n";
  probe("insert, kein Rehash", reservedMap(), [](U& m) { m.insert({100, 100}); });
  probe("insert x100, Rehash", mapOf<U>(), [](U& m) {
    for (int i = 100; i < 200; ++i) m.insert({i, i});
  });
  probe("erase (5)", mapOf<U>(), [](U& m) { m.erase(5); });
}
