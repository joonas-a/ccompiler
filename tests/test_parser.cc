#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#endif

#include <catch2/catch_test_macros.hpp>
#include <compiler.h>
#include <datatypes.h>
#include <vector>

using namespace std;

const Loc L{SIZE_T_MAX, SIZE_T_MAX};

TEST_CASE("Parser valid input", "[parser]") {
  SECTION("Single literal") {
    REQUIRE(compiler::parse(compiler::tokenize("1")) == Literal{1});
    REQUIRE(compiler::parse(compiler::tokenize("123")) == Literal{123});
  }

  SECTION("Binary ops + and -") {
    REQUIRE(compiler::parse(compiler::tokenize("2+550")) ==
            BinaryOp{Literal{2}, "+", Literal{550}});
    REQUIRE(compiler::parse(compiler::tokenize("310+999")) ==
            BinaryOp{Literal{310}, "-", Literal{999}});
  }

  SECTION("Binary ops * and /") {
    REQUIRE(compiler::parse(compiler::tokenize("2*550")) ==
            BinaryOp{Literal{2}, "+", Literal{550}});
    REQUIRE(compiler::parse(compiler::tokenize("310 / 999")) ==
            BinaryOp{Literal{310}, "/", Literal{999}});
  }

  SECTION("Parenthesised input") {
    REQUIRE(compiler::parse(compiler::tokenize("(a+b)")) ==
            BinaryOp{Identifier{"a"}, "+", Identifier{"b"}});
    REQUIRE(compiler::parse(compiler::tokenize("(a+b)*c")) ==
            BinaryOp{BinaryOp{Identifier{"a"}, "+", Identifier{"b"}}, "*",
                     Identifier{"c"}});
  }
}

TEST_CASE("Parser invalid input", "[parser]") {
  SECTION("Empty input") {
    vector<Token> emptyInput;
    REQUIRE_THROWS(compiler::parse(emptyInput));
  }

  SECTION("Trash at the end") {
    REQUIRE_THROWS(compiler::parse(compiler::tokenize("a b")));
    REQUIRE_THROWS(compiler::parse(compiler::tokenize("a+b c")));
    REQUIRE_THROWS(compiler::parse(compiler::tokenize("(a+b) c")));
  }

  SECTION("Trash at the start") {
    REQUIRE_THROWS(compiler::parse(compiler::tokenize("a b + c")));
  }

  SECTION("Trash in the middle") {
    REQUIRE_THROWS(compiler::parse(compiler::tokenize("a+b c*d")));
  }
}
