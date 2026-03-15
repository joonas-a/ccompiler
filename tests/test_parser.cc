// #ifndef CATCH_CONFIG_MAIN
// #define CATCH_CONFIG_MAIN
// #endif

// #include <catch2/catch_test_macros.hpp>
// #include <memory>
// #include <string_view>
// #include <utility>
// #include <vector>

// #include "compiler.h"
// #include "datatypes.h"
// #include "expression.h"

// using namespace compiler;
// using std::make_unique, std::vector, std::string_view;

// const Loc L{SIZE_MAX, SIZE_MAX};

// auto parse(string_view input) {
//   return compiler::parse(compiler::tokenize(input));
// }

// TEST_CASE("Parser valid input", "[parser]") {
//   SECTION("Single literal") {
//     REQUIRE(*parse("1") == *make_unique<Literal>(1UL));
//     REQUIRE(*parse("123") == *make_unique<Literal>(123UL));
//     REQUIRE(*parse("abcd") == *make_unique<Identifier>("abcd"));
//   }

//   SECTION("Binary ops +, -") {
//     REQUIRE(*parse("2+550") ==
//             *make_unique<BinaryOp>(make_unique<Literal>(2UL), "+",
//                                    make_unique<Literal>(550UL)));

//     REQUIRE(*parse("310-999") ==
//             *make_unique<BinaryOp>(make_unique<Literal>(310UL), "-",
//                                    make_unique<Literal>(999UL)));
//   }

//   SECTION("Binary ops *, %") {
//     REQUIRE(*parse("2*550") ==
//             *make_unique<BinaryOp>(make_unique<Literal>(2UL), "*",
//                                    make_unique<Literal>(550UL)));

//     REQUIRE(*parse("310 / 999") ==
//             *make_unique<BinaryOp>(make_unique<Literal>(310UL), "/",
//                                    make_unique<Literal>(999UL)));

//     REQUIRE(*parse("310 % 999") ==
//             *make_unique<BinaryOp>(make_unique<Literal>(310UL), "%",
//                                    make_unique<Literal>(999UL)));
//   }

//   SECTION("Binary ops <, <=, >, >=") {
//     REQUIRE(*parse("2<550") ==
//             *make_unique<BinaryOp>(make_unique<Literal>(2UL), "<",
//                                    make_unique<Literal>(550UL)));

//     REQUIRE(*parse("31 <= 32") ==
//             *make_unique<BinaryOp>(make_unique<Literal>(31UL),
//                                    "<=", make_unique<Literal>(32UL)));

//     REQUIRE(*parse("b>a") ==
//             *make_unique<BinaryOp>(make_unique<Identifier>("b"), ">",
//                                    make_unique<Identifier>("a")));

//     REQUIRE(*parse("b>=b") ==
//             *make_unique<BinaryOp>(make_unique<Identifier>("b"),
//                                    ">=", make_unique<Identifier>("b")));
//   }

//   SECTION("Precedence") {
//     REQUIRE(*parse("1+2*3") ==
//             *make_unique<BinaryOp>(
//                 make_unique<Literal>(1UL), "+",
//                 make_unique<BinaryOp>(make_unique<Literal>(2UL), "*",
//                                       make_unique<Literal>(3UL))));

//     REQUIRE(*parse("3*5+6") ==
//             *make_unique<BinaryOp>(
//                 make_unique<BinaryOp>(make_unique<Literal>(3UL), "*",
//                                       make_unique<Literal>(5UL)),
//                 "+", make_unique<Literal>(6UL)));

//     REQUIRE(*parse("1- -1") ==
//             *make_unique<BinaryOp>(
//                 make_unique<Literal>(1UL), "-",
//                 make_unique<UnaryOp>("-", make_unique<Literal>(1UL))));
//   }

//   SECTION("Binary ops !=, ==") {
//     REQUIRE(*parse("b != a") ==
//             *make_unique<BinaryOp>(make_unique<Identifier>("b"),
//                                    "!=", make_unique<Identifier>("a")));
//     REQUIRE(*parse("3 == 3") ==
//             *make_unique<BinaryOp>(make_unique<Literal>(3UL),
//                                    "==", make_unique<Literal>(3UL)));
//   }

//   SECTION("Binary ops and, or") {
//     REQUIRE(*parse("b and a") ==
//             *make_unique<BinaryOp>(make_unique<Identifier>("b"), "and",
//                                    make_unique<Identifier>("a")));
//     REQUIRE(*parse("c or d") ==
//             *make_unique<BinaryOp>(make_unique<Identifier>("c"), "or",
//                                    make_unique<Identifier>("d")));
//   }

//   SECTION("Unary ops -, not") {
//     REQUIRE(*parse("not 2") ==
//             *make_unique<UnaryOp>("not", make_unique<Literal>(2UL)));
//     REQUIRE(*parse("-2") ==
//             *make_unique<UnaryOp>("-", make_unique<Literal>(2UL)));
//     REQUIRE(*parse("b and not a") ==
//             *make_unique<BinaryOp>(
//                 make_unique<Identifier>("b"), "and",
//                 make_unique<UnaryOp>("not", make_unique<Identifier>("a"))));
//     REQUIRE(*parse("b and not-a") ==
//             *make_unique<BinaryOp>(
//                 make_unique<Identifier>("b"), "and",
//                 make_unique<UnaryOp>(
//                     "not",
//                     make_unique<UnaryOp>("-", make_unique<Identifier>("a")))));
//     REQUIRE(*parse("1--2") ==
//             *make_unique<BinaryOp>(
//                 make_unique<Literal>(1UL), "-",
//                 make_unique<UnaryOp>("-", make_unique<Literal>(2UL))));
//     REQUIRE(*parse("-1-1") ==
//             *make_unique<BinaryOp>(
//                 make_unique<UnaryOp>("-", make_unique<Literal>(1UL)), "-",
//                 make_unique<Literal>(1UL)));
//   }

//   SECTION("Parenthesised input") {
//     REQUIRE(*parse("(a+b)") ==
//             *make_unique<BinaryOp>(make_unique<Identifier>("a"), "+",
//                                    make_unique<Identifier>("b")));

//     REQUIRE(*parse("(a+b)*c") ==
//             *make_unique<BinaryOp>(
//                 make_unique<BinaryOp>(make_unique<Identifier>("a"), "+",
//                                       make_unique<Identifier>("b")),
//                 "*", make_unique<Identifier>("c")));
//   }

//   SECTION("If then else") {
//     REQUIRE(*parse("if a then b else c") ==
//             *make_unique<IfThenElseStatement>(make_unique<Identifier>("a"),
//                                               make_unique<Identifier>("b"),
//                                               make_unique<Identifier>("c")));
//     REQUIRE(*parse("if a then 1 + c else x * 2") ==
//             *make_unique<IfThenElseStatement>(
//                 make_unique<Identifier>("a"),
//                 make_unique<BinaryOp>(make_unique<Literal>(1UL), "+",
//                                       make_unique<Identifier>("c")),
//                 make_unique<BinaryOp>(make_unique<Identifier>("x"), "*",
//                                       make_unique<Literal>(2UL))));

//     REQUIRE(*compiler::parse(
//                 compiler::tokenize("if a then (1 + c) * 3 else x * 2")) ==
//             *make_unique<IfThenElseStatement>(
//                 make_unique<Identifier>("a"),
//                 make_unique<BinaryOp>(
//                     make_unique<BinaryOp>(make_unique<Literal>(1UL), "+",
//                                           make_unique<Identifier>("c")),
//                     "*", make_unique<Literal>(3UL)),
//                 make_unique<BinaryOp>(make_unique<Identifier>("x"), "*",
//                                       make_unique<Literal>(2UL))));
//   }

//   SECTION("Nested if else") {
//     REQUIRE(*compiler::parse(
//                 compiler::tokenize("if a then if b then 10 else d else e")) ==
//             *make_unique<IfThenElseStatement>(
//                 make_unique<Identifier>("a"),
//                 make_unique<IfThenElseStatement>(make_unique<Identifier>("b"),
//                                                  make_unique<Literal>(10UL),
//                                                  make_unique<Identifier>("d")),

//                 make_unique<Identifier>("e")));

//     REQUIRE(*compiler::parse(
//                 compiler::tokenize("if a then b else if c then d else e")) ==
//             *make_unique<IfThenElseStatement>(
//                 make_unique<Identifier>("a"), make_unique<Identifier>("b"),
//                 make_unique<IfThenElseStatement>(
//                     make_unique<Identifier>("c"), make_unique<Identifier>("d"),
//                     make_unique<Identifier>("e"))));
//   }

//   SECTION("Assignment =") {
//     REQUIRE(*parse("a = b") ==
//             *make_unique<BinaryOp>(make_unique<Identifier>("a"), "=",
//                                    make_unique<Identifier>("b")));
//     REQUIRE(*parse("a = b = c") ==
//             *make_unique<BinaryOp>(
//                 make_unique<Identifier>("a"), "=",
//                 make_unique<BinaryOp>(make_unique<Identifier>("b"), "=",
//                                       make_unique<Identifier>("c"))));
//     REQUIRE(*parse("1 * 1 = 2 % 2 = 0") ==
//             *make_unique<BinaryOp>(
//                 make_unique<BinaryOp>(make_unique<Literal>(1UL), "*",
//                                       make_unique<Literal>(1UL)),
//                 "=",
//                 make_unique<BinaryOp>(
//                     make_unique<BinaryOp>(make_unique<Literal>(2UL), "%",
//                                           make_unique<Literal>(2UL)),
//                     "=", make_unique<Literal>(0UL))));
//   }

//   SECTION("Function calls") {
//     auto args = std::vector<UPtrExpr>{};

//     args.emplace_back(std::make_unique<Literal>(1UL));
//     REQUIRE(*parse("print_int(1)") ==
//             *make_unique<FunctionCall>(make_unique<Identifier>("print_int"),
//                                        std::move(args)));

//     args.clear();
//     REQUIRE(*parse("f()") ==
//             *make_unique<FunctionCall>(make_unique<Identifier>("f"),
//                                        std::move(args)));

//     args.clear();
//     args.emplace_back(std::make_unique<BinaryOp>(
//         std::make_unique<Literal>(10UL), "*", std::make_unique<Literal>(5UL)));
//     args.emplace_back(std::make_unique<Identifier>("a"));
//     REQUIRE(*parse("plus_fifty(10*5,a)") ==
//             *make_unique<FunctionCall>(make_unique<Identifier>("plus_fifty"),
//                                        std::move(args)));
//   }

//   SECTION("Nested function call: print_int(read_int())") {
//     auto args = std::vector<UPtrExpr>{};

//     {
//       auto innerArgs = std::vector<UPtrExpr>{};
//       auto innerCall = std::make_unique<FunctionCall>(
//           std::make_unique<Identifier>("read_int"), std::move(innerArgs));

//       args.emplace_back(std::move(innerCall));
//     }

//     REQUIRE(*parse("print_int(read_int())") ==
//             *make_unique<FunctionCall>(make_unique<Identifier>("print_int"),
//                                        std::move(args)));
//   }

//   SECTION("Blocks") {
//     auto exprs = std::vector<UPtrExpr>{};
//     exprs.emplace_back(make_unique<Identifier>("a"));
//     REQUIRE(*parse("{a}") == *make_unique<Block>(std::move(exprs)));

//     exprs.clear();

//     exprs.emplace_back(make_unique<Variable>("a", make_unique<Literal>(1UL)));
//     exprs.emplace_back(make_unique<Literal>(std::monostate()));
//     REQUIRE(*parse("var a = 1;") == *make_unique<Block>(std::move(exprs)));

//     exprs.clear();

//     auto inner_exprs = std::vector<UPtrExpr>{};
//     inner_exprs.emplace_back(std::make_unique<Identifier>("a"));
//     auto inner_block = std::make_unique<Block>(std::move(inner_exprs));
//     exprs.emplace_back(std::move(inner_block));

//     REQUIRE(*parse("{{a}}") == *make_unique<Block>(std::move(exprs)));

//     exprs.clear();
//     inner_exprs.clear();
//     inner_exprs.emplace_back(std::make_unique<Identifier>("b"));
//     inner_block = std::make_unique<Block>(std::move(inner_exprs));
//     exprs.emplace_back(make_unique<Identifier>("a"));
//     exprs.emplace_back(std::move(inner_block));

//     REQUIRE(*parse("{a;{b}}") == *make_unique<Block>(std::move(exprs)));

//     exprs.clear();
//     exprs.emplace_back(make_unique<BinaryOp>(make_unique<Identifier>("a"), "+",
//                                              make_unique<Identifier>("b")));
//     exprs.emplace_back(make_unique<Literal>(std::monostate()));

//     REQUIRE(*parse("a+b;") == *make_unique<Block>(std::move(exprs)));

//     exprs.clear();
//     exprs.emplace_back(make_unique<Literal>(1UL));
//     exprs.emplace_back(make_unique<Literal>(2UL));

//     REQUIRE(*parse("1;2") == *make_unique<Block>(std::move(exprs)));

//     exprs.clear();
//     exprs.emplace_back(make_unique<Identifier>("a"));
//     exprs.emplace_back(make_unique<BinaryOp>(make_unique<Identifier>("x"), "=",
//                                              make_unique<Literal>(2UL)));
//     exprs.emplace_back(make_unique<Identifier>("z"));

//     REQUIRE(*parse("{a;x=2;z}") == *make_unique<Block>(std::move(exprs)));
//   }

//   SECTION("Variable declarations") {
//     REQUIRE(*parse("var x = 123") ==
//             *make_unique<Variable>("x", make_unique<Literal>(123UL)));

//     REQUIRE(*parse("var x = 1 + 2") ==
//             *make_unique<Variable>(
//                 "x", make_unique<BinaryOp>(make_unique<Literal>(1UL), "+",
//                                            make_unique<Literal>(2UL))));

//     auto exprs = vector<UPtrExpr>{};
//     exprs.emplace_back(make_unique<Variable>("a", make_unique<Literal>(true)));
//     exprs.emplace_back(make_unique<Variable>("b", make_unique<Literal>(false)));
//     exprs.emplace_back(make_unique<Identifier>("b"));

//     REQUIRE(*parse("var a = true; var b = false; b") ==
//             *make_unique<Block>(std::move(exprs)));
//   }

//   SECTION("While loops") {
//     REQUIRE(*parse("while 1 do 2") ==
//             *make_unique<While>(make_unique<Literal>(1UL),
//                                 make_unique<Literal>(2UL)));
//   }
// }

// TEST_CASE("Parser invalid input", "[parser]") {
//   SECTION("Empty input") {
//     vector<Token> emptyInput;
//     REQUIRE_THROWS(compiler::parse(emptyInput));
//   }

//   SECTION("Trash at the end") {
//     REQUIRE_THROWS(parse("a b"));
//     REQUIRE_THROWS(parse("a+b c"));
//     REQUIRE_THROWS(parse("(a+b) c"));
//   }

//   SECTION("Trash at the start") { REQUIRE_THROWS(parse("a b + c")); }

//   SECTION("Trash in the middle") { REQUIRE_THROWS(parse("a+b c*d")); }

//   SECTION("Unterminated if") { REQUIRE_THROWS(parse("if a")); }

//   SECTION("Else before then") { REQUIRE_THROWS(parse("if a else b")); }
//   SECTION("Unterminated function call") {
//     REQUIRE_THROWS(parse("f(a, b, c"));
//     REQUIRE_THROWS(parse("f("));
//   }
//   SECTION("Non-top level variable calls") {
//     REQUIRE_THROWS(parse("a = var b"));
//     REQUIRE_THROWS(parse("if a then var b = 1"));
//     REQUIRE_THROWS(parse("{a;b;a and var c = 1;}"));
//   }
// }

// TEST_CASE("More edge cases", "[parser-edgecase]") {
//   SECTION("Should fail") {
//     REQUIRE_THROWS(parse("{ a b }"));
//     REQUIRE_THROWS(parse("{ if true then { a } b c }"));
//   }

//   SECTION("Should parse") {
//     REQUIRE_NOTHROW(parse("{}"));
//     REQUIRE_NOTHROW(parse("{ { a } { b } }"));
//     REQUIRE_NOTHROW(parse("{ a } { b }"));
//     REQUIRE_NOTHROW(parse("if true then { a } b "));
//     REQUIRE_NOTHROW(parse("{ if true then { f(a) } d(b)}"));
//     REQUIRE_NOTHROW(parse("if true then { a } b"));

//     REQUIRE_NOTHROW(parse("{ if true then { a }; b }"));
//     REQUIRE_NOTHROW(parse("if true then { a } b; c"));
//     REQUIRE_NOTHROW(parse("{ if true then { a }; b; c }"));
//     REQUIRE_NOTHROW(parse("{ if true then { a } else { b } c }"));
//     REQUIRE_NOTHROW(parse("{ if true then { a } b; c }"));
//     REQUIRE_NOTHROW(parse("x = { { f(a) } { b } }"));

//     SECTION("Should equal") {
//       REQUIRE(*parse("{ { x }; { y } }") == *parse("{ { x } { y } }"));
//     }

//     SECTION("Should NOT equal") {
//       REQUIRE(*parse("{ { x }; { y } }") != *parse("{ { x }; { y }; }"));
//     }
//   }
// }
