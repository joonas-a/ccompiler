#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#endif

#include <catch2/catch_test_macros.hpp>

#include <string_view>

#include "compiler.h"

using std::string_view;

auto check(string_view input) {
  auto expr = compiler::parse(compiler::tokenize(input));
  return compiler::typecheck(std::move(expr));
}

TEST_CASE("Typechecker", "[typechecker]") {
  SECTION("Basic tests") {
    REQUIRE(check("1+1") == C_type::C_int);
    REQUIRE(check("if false then 1") == C_type::C_unit);
    REQUIRE(check("if true then 1 else 2") == C_type::C_int);
    REQUIRE(check("if true then true else false") == C_type::C_bool);
    REQUIRE(check("var a = 5") == C_type::C_unit);
    REQUIRE(check("var b = 5; b + 2") == C_type::C_int);
    REQUIRE(check("{ { var b = 5; b + 2 } }") == C_type::C_int);
    REQUIRE(check("if 1<2 then 1") == C_type::C_unit);
    REQUIRE(check("1 < 2") == C_type::C_bool);
    REQUIRE(check("1 == 2") == C_type::C_bool);
    REQUIRE(check("true == true") == C_type::C_bool);
    REQUIRE(check("true == false") == C_type::C_bool);
  }

  SECTION("Should not pass") {
    REQUIRE_THROWS(check("{var a = 10}; a + 10"));
    REQUIRE_THROWS(check("true == 1"));
    REQUIRE_THROWS(check("1 == false"));
  }
}
