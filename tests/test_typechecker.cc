#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#include <variant>
#endif

#include <catch2/catch_test_macros.hpp>

#include <string_view>

#include "compiler.h"
#include "typecheck.h"

using std::string_view, std::holds_alternative;

auto check(string_view input) {
  auto expr = compiler::parse(compiler::tokenize(input));
  return compiler::typecheck(std::move(expr));
}

TEST_CASE("Typechecker", "[typechecker]") {
  SECTION("Basic tests") {
    REQUIRE(holds_alternative<C_int>(check("1+1")));
    REQUIRE(holds_alternative<C_unit>(check("if false then 1")));
    REQUIRE(holds_alternative<C_int>(check("if true then 1 else 2")));
    REQUIRE(holds_alternative<C_bool>(check("if true then true else false")));
    REQUIRE(holds_alternative<C_unit>(check("var a = 5")));
    REQUIRE(holds_alternative<C_int>(check("var b = 5; b + 2")));
    REQUIRE(holds_alternative<C_int>(check("{ { var b = 5; b + 2 } }")));
    REQUIRE(holds_alternative<C_unit>(check("if 1<2 then 1")));
    REQUIRE(holds_alternative<C_bool>(check("1 < 2")));
    REQUIRE(holds_alternative<C_bool>(check("1 == 2")));
    REQUIRE(holds_alternative<C_bool>(check("true == true")));
    REQUIRE(holds_alternative<C_bool>(check("true == false")));
  }

  SECTION("Should not pass") {
    REQUIRE_THROWS(check("{var a = 10}; a + 10"));
    REQUIRE_THROWS(check("true == 1"));
    REQUIRE_THROWS(check("1 == false"));
    REQUIRE_THROWS(check("var a: Bool  = 1;"));
    REQUIRE_THROWS(check("var a: Int = print_int;"));
    REQUIRE_THROWS(check("var x: (true) => Unit = print_int; x(4)"));
  }

  SECTION("Typehints") {
    REQUIRE_NOTHROW(check("var x: Int = 3; x + 1"));
    REQUIRE_NOTHROW(check("var x: Bool = 1 < 2; x"));
    REQUIRE_NOTHROW(check("var x: (Bool) => Unit = print_bool; x(true)"));
    REQUIRE_NOTHROW(check("var x: (Int) => Unit = print_int; x(4)"));
  }
}
