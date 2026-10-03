#include <cstddef>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

int main() {
  std::string text = "the car stops at the red light and the driver waits "
                     "the light turns green and the car drives on "
                     "the driver checks the mirror and the car turns left";

  std::map<std::string, int> wordcount;

  std::istringstream iss{text};
  std::string word;
  while (iss >> word) {
    // works
    // ++wordCount[word]

    // Option 2
    // if (auto it = wordcount.find(word); it != wordcount.end()) {
    //   (it->second)++;
    // } else {
    //   wordcount.insert({word, 1});
    // }

    auto [it, _] = wordcount.insert({word, 0});
    ++it->second;
  }

  for (auto& [word, count] : wordcount) {
    std::cout << word << ": " << count << std::endl;
  };

  std::cout << "Hello World!" << std::endl;
  return 0;
}