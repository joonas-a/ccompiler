#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#include <memory>
#endif

#include <catch2/catch_test_macros.hpp>
#include <compiler.h>
#include <datatypes.h>
#include <vector>

using namespace std;

const Loc L{SIZE_T_MAX, SIZE_T_MAX};

TEST_CASE("Parser valid input", "[parser]") {
  SECTION("Single literal") {
    REQUIRE(*compiler::parse(compiler::tokenize("1")) ==
            *make_unique<Literal>(1));
    REQUIRE(*compiler::parse(compiler::tokenize("123")) ==
            *make_unique<Literal>(123));
    REQUIRE(*compiler::parse(compiler::tokenize("abcd")) ==
            *make_unique<Identifier>("abcd"));
  }

  SECTION("Binary ops + and -") {
    REQUIRE(*compiler::parse(compiler::tokenize("2+550")) ==
            *make_unique<BinaryOp>(make_unique<Literal>(2), "+",
                                   make_unique<Literal>(550)));

    REQUIRE(*compiler::parse(compiler::tokenize("310-999")) ==
            *make_unique<BinaryOp>(make_unique<Literal>(310), "-",
                                   make_unique<Literal>(999)));
  }

  SECTION("Binary ops * and /") {
    REQUIRE(*compiler::parse(compiler::tokenize("2*550")) ==
            *make_unique<BinaryOp>(make_unique<Literal>(2), "*",
                                   make_unique<Literal>(550)));

    REQUIRE(*compiler::parse(compiler::tokenize("310 / 999")) ==
            *make_unique<BinaryOp>(make_unique<Literal>(310), "/",
                                   make_unique<Literal>(999)));
  }

  SECTION("Parenthesised input") {
    REQUIRE(*compiler::parse(compiler::tokenize("(a+b)")) ==
            *make_unique<BinaryOp>(make_unique<Identifier>("a"), "+",
                                   make_unique<Identifier>("b")));

    REQUIRE(*compiler::parse(compiler::tokenize("(a+b)*c")) ==
            *make_unique<BinaryOp>(
                make_unique<BinaryOp>(make_unique<Identifier>("a"), "+",
                                      make_unique<Identifier>("b")),
                "*", make_unique<Identifier>("c")));
  }

  SECTION("If then else") {
    REQUIRE(*compiler::parse(compiler::tokenize("if a then b else c")) ==
            *make_unique<IfThenElseStatement>(make_unique<Identifier>("a"),
                                              make_unique<Identifier>("b"),
                                              make_unique<Identifier>("c")));
    REQUIRE(
        *compiler::parse(compiler::tokenize("if a then 1 + c else x * 2")) ==
        *make_unique<IfThenElseStatement>(
            make_unique<Identifier>("a"),
            make_unique<BinaryOp>(make_unique<Literal>(1), "+",
                                  make_unique<Identifier>("c")),
            make_unique<BinaryOp>(make_unique<Identifier>("x"), "*",
                                  make_unique<Literal>(2))));

    REQUIRE(*compiler::parse(
                compiler::tokenize("if a then (1 + c) * 3 else x * 2")) ==
            *make_unique<IfThenElseStatement>(
                make_unique<Identifier>("a"),
                make_unique<BinaryOp>(
                    make_unique<BinaryOp>(make_unique<Literal>(1), "+",
                                          make_unique<Identifier>("c")),
                    "*", make_unique<Literal>(3)),
                make_unique<BinaryOp>(make_unique<Identifier>("x"), "*",
                                      make_unique<Literal>(2))));
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
