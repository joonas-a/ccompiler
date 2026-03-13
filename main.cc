#include <iostream>
#include <iterator>
#include <string>

#include "compiler.h"

static auto generate_assembly(std::string_view input) {
  auto ast = compiler::parse(compiler::tokenize(input));
  auto root_type = typecheck(std::move(ast));
  generate_assembly(generate_ir(ast, root_type));
}

int main(int argc, char *argv[]) {

  std::string input((std::istreambuf_iterator<char>(std::cin)),
                    std::istreambuf_iterator<char>());

  // NB: Assembly generator outputs to stdout on its own
  generate_assembly(input);

  return 0;
}
