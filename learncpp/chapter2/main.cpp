#include "io.hpp"

int main() {
  int a{readNumber()};
  int b{readNumber()};

  writeAnswer(a + b);
  return 0;
}