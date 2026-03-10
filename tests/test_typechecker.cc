#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#endif

#include <catch2/catch_test_macros.hpp>

#include <string_view>

#include "compiler.h"
#include "typecheck.h"

using std::string_view;

auto check(string_view input) {
  auto expr = compiler::parse(compiler::tokenize(input));
  return compiler::typecheck(expr);
}

TEST_CASE("Typechecker", "[typechecker]") {
  SECTION("Basic tests") {
    REQUIRE(check("1+1") == C_type::C_int);
    REQUIRE(check("if false then 1") == C_type::C_unit);
    REQUIRE(check("if true then 1 else 2") == C_type::C_int);
    REQUIRE(check("if true then true else false") == C_type::C_bool);
    REQUIRE(check("var a = 5") == C_type::C_unit);
    REQUIRE(check("var b = 5; b + 2") == C_type::C_int);
  }
}
