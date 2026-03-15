#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#endif

#include <catch2/catch_test_macros.hpp>

#include <string_view>

#include "compiler.h"

namespace compiler {

auto make_asm(std::string_view input) {
  auto ast = parse(tokenize(input));
  auto root_type = typecheck(std::move(ast));

  return generate_assembly(generate_ir(ast, root_type));
}

// TEST_CASE("Assembly generator works", "[assembly]") {
//   SECTION("Basic tests") { REQUIRE_NOTHROW(make_asm("1+1")); }
// }

TEST_CASE("Assembly more tests", "[assembly]") {
  SECTION("Variable fn assignment") {
    REQUIRE_NOTHROW(make_asm("var x = print_int; x(4)"));
  }
}
} // namespace compiler
