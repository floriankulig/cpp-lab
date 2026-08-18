#include <iostream>

int readNumber() {
  std::cout << "Enter a number: " << std::endl;
  int readNum{};
  std::cin >> readNum;
  return readNum;
}

void writeAnswer(int ans) { std::cout << "Answer: " << ans << std::endl; }