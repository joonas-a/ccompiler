#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#include <memory>
#endif

#include <catch2/catch_test_macros.hpp>
#include <compiler.h>
#include <datatypes.h>
#include <utility>
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

  SECTION("Binary ops +, -") {
    REQUIRE(*compiler::parse(compiler::tokenize("2+550")) ==
            *make_unique<BinaryOp>(make_unique<Literal>(2), "+",
                                   make_unique<Literal>(550)));

    REQUIRE(*compiler::parse(compiler::tokenize("310-999")) ==
            *make_unique<BinaryOp>(make_unique<Literal>(310), "-",
                                   make_unique<Literal>(999)));
  }

  SECTION("Binary ops *, %") {
    REQUIRE(*compiler::parse(compiler::tokenize("2*550")) ==
            *make_unique<BinaryOp>(make_unique<Literal>(2), "*",
                                   make_unique<Literal>(550)));

    REQUIRE(*compiler::parse(compiler::tokenize("310 / 999")) ==
            *make_unique<BinaryOp>(make_unique<Literal>(310), "/",
                                   make_unique<Literal>(999)));

    REQUIRE(*compiler::parse(compiler::tokenize("310 % 999")) ==
            *make_unique<BinaryOp>(make_unique<Literal>(310), "%",
                                   make_unique<Literal>(999)));
  }

  SECTION("Binary ops <, <=, >, >=") {
    REQUIRE(*compiler::parse(compiler::tokenize("2<550")) ==
            *make_unique<BinaryOp>(make_unique<Literal>(2), "<",
                                   make_unique<Literal>(550)));

    REQUIRE(*compiler::parse(compiler::tokenize("31 <= 32")) ==
            *make_unique<BinaryOp>(make_unique<Literal>(31),
                                   "<=", make_unique<Literal>(32)));

    REQUIRE(*compiler::parse(compiler::tokenize("b>a")) ==
            *make_unique<BinaryOp>(make_unique<Identifier>("b"), ">",
                                   make_unique<Identifier>("a")));

    REQUIRE(*compiler::parse(compiler::tokenize("b>=b")) ==
            *make_unique<BinaryOp>(make_unique<Identifier>("b"),
                                   ">=", make_unique<Identifier>("b")));
  }

  SECTION("Binary ops !=, ==") {
    REQUIRE(*compiler::parse(compiler::tokenize("b != a")) ==
            *make_unique<BinaryOp>(make_unique<Identifier>("b"),
                                   "!=", make_unique<Identifier>("a")));
    REQUIRE(*compiler::parse(compiler::tokenize("3 == 3")) ==
            *make_unique<BinaryOp>(make_unique<Literal>(3),
                                   "==", make_unique<Literal>(3)));
  }

  SECTION("Binary ops and, or") {
    REQUIRE(*compiler::parse(compiler::tokenize("b and a")) ==
            *make_unique<BinaryOp>(make_unique<Identifier>("b"), "and",
                                   make_unique<Identifier>("a")));
    REQUIRE(*compiler::parse(compiler::tokenize("c or d")) ==
            *make_unique<BinaryOp>(make_unique<Identifier>("c"), "or",
                                   make_unique<Identifier>("d")));
  }

  SECTION("Unary ops -, not") {
    REQUIRE(*compiler::parse(compiler::tokenize("not 2")) ==
            *make_unique<UnaryOp>("not", make_unique<Literal>(2)));
    REQUIRE(*compiler::parse(compiler::tokenize("- 2")) ==
            *make_unique<UnaryOp>("-", make_unique<Literal>(2)));
    REQUIRE(*compiler::parse(compiler::tokenize("b and not a")) ==
            *make_unique<BinaryOp>(
                make_unique<Identifier>("b"), "and",
                make_unique<UnaryOp>("not", make_unique<Identifier>("a"))));
    REQUIRE(*compiler::parse(compiler::tokenize("b and not-a")) ==
            *make_unique<BinaryOp>(
                make_unique<Identifier>("b"), "and",
                make_unique<UnaryOp>(
                    "not",
                    make_unique<UnaryOp>("-", make_unique<Identifier>("a")))));
    REQUIRE(*compiler::parse(compiler::tokenize("1--2")) ==
            *make_unique<BinaryOp>(
                make_unique<Literal>(1), "-",
                make_unique<UnaryOp>("-", make_unique<Literal>(2))));
    REQUIRE(*compiler::parse(compiler::tokenize("-1-1")) ==
            *make_unique<BinaryOp>(
                make_unique<UnaryOp>("-", make_unique<Literal>(1)), "-",
                make_unique<Literal>(1)));
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
    REQUIRE(*compiler::parse(compiler::tokenize("a = b")) ==
            *make_unique<BinaryOp>(make_unique<Identifier>("a"), "=",
                                   make_unique<Identifier>("b")));
    REQUIRE(*compiler::parse(compiler::tokenize("a = b = c")) ==
            *make_unique<BinaryOp>(
                make_unique<Identifier>("a"), "=",
                make_unique<BinaryOp>(make_unique<Identifier>("b"), "=",
                                      make_unique<Identifier>("c"))));
    REQUIRE(*compiler::parse(compiler::tokenize("1 * 1 = 2 % 2 = 0")) ==
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
    REQUIRE(*compiler::parse(compiler::tokenize("f(a)")) ==
            *make_unique<FunctionCall>(make_unique<Identifier>("f"),
                                       std::move(args)));

    args.clear();
    REQUIRE(*compiler::parse(compiler::tokenize("f()")) ==
            *make_unique<FunctionCall>(make_unique<Identifier>("f"),
                                       std::move(args)));

    args.clear();
    args.emplace_back(std::make_unique<BinaryOp>(
        std::make_unique<Literal>(10), "*", std::make_unique<Literal>(5)));
    args.emplace_back(std::make_unique<Identifier>("a"));
    REQUIRE(*compiler::parse(compiler::tokenize("plus_fifty(10*5,a)")) ==
            *make_unique<FunctionCall>(make_unique<Identifier>("plus_fifty"),
                                       std::move(args)));
  }

  SECTION("Blocks") {
    auto exprs = std::vector<std::unique_ptr<Expression>>{};
    exprs.emplace_back(make_unique<Identifier>("a"));
    auto block = make_unique<Block>(std::move(exprs));

    REQUIRE(*compiler::parse(compiler::tokenize("{a}")) == *block);

    exprs.clear();
    auto inner_exprs = std::vector<std::unique_ptr<Expression>>{};
    inner_exprs.emplace_back(std::make_unique<Identifier>("a"));
    auto inner_block = std::make_unique<Block>(std::move(inner_exprs));
    exprs.emplace_back(std::move(inner_block));

    REQUIRE(*compiler::parse(compiler::tokenize("{{a}}")) ==
            *make_unique<Block>(std::move(exprs)));

    exprs.clear();
    inner_exprs.clear();
    inner_exprs.emplace_back(std::make_unique<Identifier>("b"));
    inner_block = std::make_unique<Block>(std::move(inner_exprs));
    exprs.emplace_back(make_unique<Identifier>("a"));
    exprs.emplace_back(std::move(inner_block));

    REQUIRE(*compiler::parse(compiler::tokenize("{a;{b}}")) ==
            *make_unique<Block>(std::move(exprs)));

    exprs.clear();
    exprs.emplace_back(make_unique<BinaryOp>(make_unique<Identifier>("a"), "+",
                                             make_unique<Identifier>("b")));
    exprs.emplace_back(make_unique<Literal>(std::monostate()));

    REQUIRE(*compiler::parse(compiler::tokenize("a+b;")) ==
            *make_unique<Block>(std::move(exprs)));

    exprs.clear();
    exprs.emplace_back(make_unique<Literal>(1));
    exprs.emplace_back(make_unique<Literal>(2));

    REQUIRE(*compiler::parse(compiler::tokenize("1;2")) ==
            *make_unique<Block>(std::move(exprs)));

    exprs.clear();
    exprs.emplace_back(make_unique<Identifier>("a"));
    exprs.emplace_back(make_unique<BinaryOp>(make_unique<Identifier>("x"), "=",
                                             make_unique<Literal>(2)));
    exprs.emplace_back(make_unique<Identifier>("z"));

    REQUIRE(*compiler::parse(compiler::tokenize("{a;x=2;z}")) ==
            *make_unique<Block>(std::move(exprs)));
  }

  SECTION("Variable declarations") {
    REQUIRE(*compiler::parse(compiler::tokenize("var x = 123")) ==
            *make_unique<Variable>("x", make_unique<Literal>(123)));

    REQUIRE(*compiler::parse(compiler::tokenize("var x = 1 + 2")) ==
            *make_unique<Variable>(
                "x", make_unique<BinaryOp>(make_unique<Literal>(1), "+",
                                           make_unique<Literal>(2))));
  }

  SECTION("While loops") {
    REQUIRE(
        *compiler::parse(compiler::tokenize("while 1 do 2")) ==
        *make_unique<While>(make_unique<Literal>(1), make_unique<Literal>(2)));
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

  SECTION("Unterminated if") {
    REQUIRE_THROWS(compiler::parse(compiler::tokenize("if a")));
  }

  SECTION("Else before then") {
    REQUIRE_THROWS(compiler::parse(compiler::tokenize("if a else b")));
  }
  SECTION("Unterminated function call") {
    REQUIRE_THROWS(compiler::parse(compiler::tokenize("f(a, b, c")));
    REQUIRE_THROWS(compiler::parse(compiler::tokenize("f(")));
  }
  SECTION("Non-top level variable calls") {
    REQUIRE_THROWS(compiler::parse(compiler::tokenize("a = var b")));
    REQUIRE_THROWS(compiler::parse(compiler::tokenize("if a then var b = 1")));
    REQUIRE_THROWS(
        compiler::parse(compiler::tokenize("{a;b;a and var c = 1;}")));
  }
}

TEST_CASE("More edge cases", "[parser-edgecase]") {
  SECTION("Should fail") {
    REQUIRE_THROWS(compiler::parse(compiler::tokenize("{ a b }")));
    REQUIRE_THROWS(
        compiler::parse(compiler::tokenize("{ if true then { a } b c }")));
  }

  SECTION("Should parse") {
    REQUIRE_NOTHROW(compiler::parse(compiler::tokenize("{}")));
    REQUIRE_NOTHROW(compiler::parse(compiler::tokenize("{ { a } { b } }")));
    REQUIRE_NOTHROW(compiler::parse(compiler::tokenize("{ a } { b }")));
    REQUIRE_NOTHROW(
        compiler::parse(compiler::tokenize("if true then { a } b ")));
    // REQUIRE_NOTHROW(
    //     compiler::parse(compiler::tokenize("{ if true then { f(a) } d(b)
    //     }")));
    REQUIRE_NOTHROW(
        compiler::parse(compiler::tokenize("if true then { a } b")));

    REQUIRE_NOTHROW(
        compiler::parse(compiler::tokenize("{ if true then { a }; b }")));
    REQUIRE_NOTHROW(
        compiler::parse(compiler::tokenize("if true then { a } b; c")));
    // REQUIRE_NOTHROW(
    //     compiler::parse(compiler::tokenize("{ if true then { a }; b; c }")));
    // REQUIRE_NOTHROW(compiler::parse(
    //     compiler::tokenize("{ if true then { a } else { b } c }")));
    REQUIRE_NOTHROW(
        compiler::parse(compiler::tokenize("x = { { f(a) } { b } }")));

    SECTION("Should equal") {
      REQUIRE(*compiler::parse(compiler::tokenize("{ { x }; { y } }")) ==
              *compiler::parse(compiler::tokenize("{ { x } { y } }")));
    }

    SECTION("Should NOT equal") {
      REQUIRE(*compiler::parse(compiler::tokenize("{ { x }; { y } }")) !=
              *compiler::parse(compiler::tokenize("{ { x }; { y }; }")));
    }
  }
}
