#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#include <string_view>
#endif

#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <utility>
#include <vector>

#include "compiler.h"
#include "datatypes.h"

using namespace compiler;
using std::make_unique, std::vector, std::string_view;

const Loc L{SIZE_MAX, SIZE_MAX};

auto parse(string_view input) {
  return compiler::parse(compiler::tokenize(input));
}

TEST_CASE("Parser valid input", "[parser]") {
  SECTION("Single literal") {
    REQUIRE(*parse("1") == *make_unique<Literal>(1));
    REQUIRE(*parse("123") == *make_unique<Literal>(123));
    REQUIRE(*parse("abcd") == *make_unique<Identifier>("abcd"));
  }

  SECTION("Binary ops +, -") {
    REQUIRE(*parse("2+550") ==
            *make_unique<BinaryOp>(make_unique<Literal>(2), "+",
                                   make_unique<Literal>(550)));

    REQUIRE(*parse("310-999") ==
            *make_unique<BinaryOp>(make_unique<Literal>(310), "-",
                                   make_unique<Literal>(999)));
  }

  SECTION("Binary ops *, %") {
    REQUIRE(*parse("2*550") ==
            *make_unique<BinaryOp>(make_unique<Literal>(2), "*",
                                   make_unique<Literal>(550)));

    REQUIRE(*parse("310 / 999") ==
            *make_unique<BinaryOp>(make_unique<Literal>(310), "/",
                                   make_unique<Literal>(999)));

    REQUIRE(*parse("310 % 999") ==
            *make_unique<BinaryOp>(make_unique<Literal>(310), "%",
                                   make_unique<Literal>(999)));
  }

  SECTION("Binary ops <, <=, >, >=") {
    REQUIRE(*parse("2<550") ==
            *make_unique<BinaryOp>(make_unique<Literal>(2), "<",
                                   make_unique<Literal>(550)));

    REQUIRE(*parse("31 <= 32") ==
            *make_unique<BinaryOp>(make_unique<Literal>(31),
                                   "<=", make_unique<Literal>(32)));

    REQUIRE(*parse("b>a") ==
            *make_unique<BinaryOp>(make_unique<Identifier>("b"), ">",
                                   make_unique<Identifier>("a")));

    REQUIRE(*parse("b>=b") ==
            *make_unique<BinaryOp>(make_unique<Identifier>("b"),
                                   ">=", make_unique<Identifier>("b")));
  }

  SECTION("Binary ops !=, ==") {
    REQUIRE(*parse("b != a") ==
            *make_unique<BinaryOp>(make_unique<Identifier>("b"),
                                   "!=", make_unique<Identifier>("a")));
    REQUIRE(*parse("3 == 3") ==
            *make_unique<BinaryOp>(make_unique<Literal>(3),
                                   "==", make_unique<Literal>(3)));
  }

  SECTION("Binary ops and, or") {
    REQUIRE(*parse("b and a") ==
            *make_unique<BinaryOp>(make_unique<Identifier>("b"), "and",
                                   make_unique<Identifier>("a")));
    REQUIRE(*parse("c or d") ==
            *make_unique<BinaryOp>(make_unique<Identifier>("c"), "or",
                                   make_unique<Identifier>("d")));
  }

  SECTION("Unary ops -, not") {
    REQUIRE(*parse("not 2") ==
            *make_unique<UnaryOp>("not", make_unique<Literal>(2)));
    REQUIRE(*parse("- 2") ==
            *make_unique<UnaryOp>("-", make_unique<Literal>(2)));
    REQUIRE(*parse("b and not a") ==
            *make_unique<BinaryOp>(
                make_unique<Identifier>("b"), "and",
                make_unique<UnaryOp>("not", make_unique<Identifier>("a"))));
    REQUIRE(*parse("b and not-a") ==
            *make_unique<BinaryOp>(
                make_unique<Identifier>("b"), "and",
                make_unique<UnaryOp>(
                    "not",
                    make_unique<UnaryOp>("-", make_unique<Identifier>("a")))));
    REQUIRE(*parse("1--2") ==
            *make_unique<BinaryOp>(
                make_unique<Literal>(1), "-",
                make_unique<UnaryOp>("-", make_unique<Literal>(2))));
    REQUIRE(*parse("-1-1") ==
            *make_unique<BinaryOp>(
                make_unique<UnaryOp>("-", make_unique<Literal>(1)), "-",
                make_unique<Literal>(1)));
  }

  SECTION("Parenthesised input") {
    REQUIRE(*parse("(a+b)") ==
            *make_unique<BinaryOp>(make_unique<Identifier>("a"), "+",
                                   make_unique<Identifier>("b")));

    REQUIRE(*parse("(a+b)*c") ==
            *make_unique<BinaryOp>(
                make_unique<BinaryOp>(make_unique<Identifier>("a"), "+",
                                      make_unique<Identifier>("b")),
                "*", make_unique<Identifier>("c")));
  }

  SECTION("If then else") {
    REQUIRE(*parse("if a then b else c") ==
            *make_unique<IfThenElseStatement>(make_unique<Identifier>("a"),
                                              make_unique<Identifier>("b"),
                                              make_unique<Identifier>("c")));
    REQUIRE(*parse("if a then 1 + c else x * 2") ==
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

  SECTION("Nested if else") {
    REQUIRE(*compiler::parse(
                compiler::tokenize("if a then if b then 10 else d else e")) ==
            *make_unique<IfThenElseStatement>(
                make_unique<Identifier>("a"),
                make_unique<IfThenElseStatement>(make_unique<Identifier>("b"),
                                                 make_unique<Literal>(10),
                                                 make_unique<Identifier>("d")),

                make_unique<Identifier>("e")));

    REQUIRE(*compiler::parse(
                compiler::tokenize("if a then b else if c then d else e")) ==
            *make_unique<IfThenElseStatement>(
                make_unique<Identifier>("a"), make_unique<Identifier>("b"),
                make_unique<IfThenElseStatement>(
                    make_unique<Identifier>("c"), make_unique<Identifier>("d"),
                    make_unique<Identifier>("e"))));
  }

  SECTION("Assignment =") {
    REQUIRE(*parse("a = b") ==
            *make_unique<BinaryOp>(make_unique<Identifier>("a"), "=",
                                   make_unique<Identifier>("b")));
    REQUIRE(*parse("a = b = c") ==
            *make_unique<BinaryOp>(
                make_unique<Identifier>("a"), "=",
                make_unique<BinaryOp>(make_unique<Identifier>("b"), "=",
                                      make_unique<Identifier>("c"))));
    REQUIRE(*parse("1 * 1 = 2 % 2 = 0") ==
            *make_unique<BinaryOp>(
                make_unique<BinaryOp>(make_unique<Literal>(1), "*",
                                      make_unique<Literal>(1)),
                "=",
                make_unique<BinaryOp>(
                    make_unique<BinaryOp>(make_unique<Literal>(2), "%",
                                          make_unique<Literal>(2)),
                    "=", make_unique<Literal>(0))));
  }

  SECTION("Function calls") {
    auto args = std::vector<std::unique_ptr<Expression>>{};

    args.emplace_back(std::make_unique<Identifier>("a"));
    REQUIRE(*parse("f(a)") ==
            *make_unique<FunctionCall>(make_unique<Identifier>("f"),
                                       std::move(args)));

    args.clear();
    REQUIRE(*parse("f()") ==
            *make_unique<FunctionCall>(make_unique<Identifier>("f"),
                                       std::move(args)));

    args.clear();
    args.emplace_back(std::make_unique<BinaryOp>(
        std::make_unique<Literal>(10), "*", std::make_unique<Literal>(5)));
    args.emplace_back(std::make_unique<Identifier>("a"));
    REQUIRE(*parse("plus_fifty(10*5,a)") ==
            *make_unique<FunctionCall>(make_unique<Identifier>("plus_fifty"),
                                       std::move(args)));
  }

  SECTION("Blocks") {
    auto exprs = std::vector<std::unique_ptr<Expression>>{};
    exprs.emplace_back(make_unique<Identifier>("a"));
    auto block = make_unique<Block>(std::move(exprs));

    REQUIRE(*parse("{a}") == *block);

    exprs.clear();
    auto inner_exprs = std::vector<std::unique_ptr<Expression>>{};
    inner_exprs.emplace_back(std::make_unique<Identifier>("a"));
    auto inner_block = std::make_unique<Block>(std::move(inner_exprs));
    exprs.emplace_back(std::move(inner_block));

    REQUIRE(*parse("{{a}}") == *make_unique<Block>(std::move(exprs)));

    exprs.clear();
    inner_exprs.clear();
    inner_exprs.emplace_back(std::make_unique<Identifier>("b"));
    inner_block = std::make_unique<Block>(std::move(inner_exprs));
    exprs.emplace_back(make_unique<Identifier>("a"));
    exprs.emplace_back(std::move(inner_block));

    REQUIRE(*parse("{a;{b}}") == *make_unique<Block>(std::move(exprs)));

    exprs.clear();
    exprs.emplace_back(make_unique<BinaryOp>(make_unique<Identifier>("a"), "+",
                                             make_unique<Identifier>("b")));
    exprs.emplace_back(make_unique<Literal>(std::monostate()));

    REQUIRE(*parse("a+b;") == *make_unique<Block>(std::move(exprs)));

    exprs.clear();
    exprs.emplace_back(make_unique<Literal>(1));
    exprs.emplace_back(make_unique<Literal>(2));

    REQUIRE(*parse("1;2") == *make_unique<Block>(std::move(exprs)));

    exprs.clear();
    exprs.emplace_back(make_unique<Identifier>("a"));
    exprs.emplace_back(make_unique<BinaryOp>(make_unique<Identifier>("x"), "=",
                                             make_unique<Literal>(2)));
    exprs.emplace_back(make_unique<Identifier>("z"));

    REQUIRE(*parse("{a;x=2;z}") == *make_unique<Block>(std::move(exprs)));
  }

  SECTION("Variable declarations") {
    REQUIRE(*parse("var x = 123") ==
            *make_unique<Variable>("x", make_unique<Literal>(123)));

    REQUIRE(*parse("var x = 1 + 2") ==
            *make_unique<Variable>(
                "x", make_unique<BinaryOp>(make_unique<Literal>(1), "+",
                                           make_unique<Literal>(2))));
  }

  SECTION("While loops") {
    REQUIRE(
        *parse("while 1 do 2") ==
        *make_unique<While>(make_unique<Literal>(1), make_unique<Literal>(2)));
  }
}

TEST_CASE("Parser invalid input", "[parser]") {
  SECTION("Empty input") {
    vector<Token> emptyInput;
    REQUIRE_THROWS(compiler::parse(emptyInput));
  }

  SECTION("Trash at the end") {
    REQUIRE_THROWS(parse("a b"));
    REQUIRE_THROWS(parse("a+b c"));
    REQUIRE_THROWS(parse("(a+b) c"));
  }

  SECTION("Trash at the start") { REQUIRE_THROWS(parse("a b + c")); }

  SECTION("Trash in the middle") { REQUIRE_THROWS(parse("a+b c*d")); }

  SECTION("Unterminated if") { REQUIRE_THROWS(parse("if a")); }

  SECTION("Else before then") { REQUIRE_THROWS(parse("if a else b")); }
  SECTION("Unterminated function call") {
    REQUIRE_THROWS(parse("f(a, b, c"));
    REQUIRE_THROWS(parse("f("));
  }
  SECTION("Non-top level variable calls") {
    REQUIRE_THROWS(parse("a = var b"));
    REQUIRE_THROWS(parse("if a then var b = 1"));
    REQUIRE_THROWS(parse("{a;b;a and var c = 1;}"));
  }
}

TEST_CASE("More edge cases", "[parser-edgecase]") {
  SECTION("Should fail") {
    REQUIRE_THROWS(parse("{ a b }"));
    REQUIRE_THROWS(parse("{ if true then { a } b c }"));
  }

  SECTION("Should parse") {
    REQUIRE_NOTHROW(parse("{}"));
    REQUIRE_NOTHROW(parse("{ { a } { b } }"));
    REQUIRE_NOTHROW(parse("{ a } { b }"));
    REQUIRE_NOTHROW(parse("if true then { a } b "));
    // REQUIRE_NOTHROW(
    //     helper("{ if true then { f(a } d(b)
    //     }")));
    REQUIRE_NOTHROW(parse("if true then { a } b"));

    REQUIRE_NOTHROW(parse("{ if true then { a }; b }"));
    REQUIRE_NOTHROW(parse("if true then { a } b; c"));
    // REQUIRE_NOTHROW(
    //     parse("{ if true then { a }; b; c }"));
    // REQUIRE_NOTHROW(compiler::parse(
    //     compiler::tokenize("{ if true then { a } else { b } c }")));
    // REQUIRE_NOTHROW(parse("{ if true then { a } b; c }")); // TODO: fix test
    REQUIRE_NOTHROW(parse("x = { { f(a) } { b } }"));

    SECTION("Should equal") {
      REQUIRE(*parse("{ { x }; { y } }") == *parse("{ { x } { y } }"));
    }

    SECTION("Should NOT equal") {
      REQUIRE(*parse("{ { x }; { y } }") != *parse("{ { x }; { y }; }"));
    }
  }
}
