#include <iostream>
#include <string>

const std::string HOST = "127.0.0.1";
constexpr int PORT = 3000;

int main(int argc, char *argv[]) {

  std::cout << argc << " " << argv << std::endl;

  int i = 1;

  for (; i < argc; i++) {
    std::cout << argv[i] << std::endl;
  }
  return 0;
}
