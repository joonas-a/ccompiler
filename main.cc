#include <iostream>

int main(int argc, char *argv[]) {

  std::cout << argc << " " << argv << std::endl;

  int i = 1;

  for (; i < argc; i++) {
    std::cout << argv[i] << std::endl;
  }
  return 0;
}
