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
    const vector<Token> literal_int_1{Token{L, Kind::INT_LITERAL, "1"}};
    const vector<Token> literal_int_2{Token{L, Kind::INT_LITERAL, "123"}};

    REQUIRE(compiler::parse(literal_int_1) == Literal{1});
    REQUIRE(compiler::parse(literal_int_2) == Literal{123});
  }

  SECTION("Binary ops + and -") {
    vector<Token> input1{
        Token({L, Kind::INT_LITERAL, "2"}),
        Token({L, Kind::OPERATOR, "+"}),
        Token({L, Kind::INT_LITERAL, "550"}),
    };
    vector<Token> input2{
        Token({L, Kind::INT_LITERAL, "310"}),
        Token({L, Kind::OPERATOR, "-"}),
        Token({L, Kind::INT_LITERAL, "999"}),
    };

    REQUIRE(compiler::parse(input1) == BinaryOp{Literal{2}, "+", Literal{550}});
    REQUIRE(compiler::parse(input2) ==
            BinaryOp{Literal{310}, "-", Literal{999}});
  }

  SECTION("Binary ops * and /") {
    vector<Token> input1{
        Token({L, Kind::INT_LITERAL, "2"}),
        Token({L, Kind::OPERATOR, "*"}),
        Token({L, Kind::INT_LITERAL, "550"}),
    };
    vector<Token> input2{
        Token({L, Kind::INT_LITERAL, "310"}),
        Token({L, Kind::OPERATOR, "/"}),
        Token({L, Kind::INT_LITERAL, "999"}),
    };

    REQUIRE(compiler::parse(input1) == BinaryOp{Literal{2}, "+", Literal{550}});
    REQUIRE(compiler::parse(input2) ==
            BinaryOp{Literal{310}, "/", Literal{999}});
  }

  SECTION("Parenthesised input") {
    vector<Token> input1{
        Token({L, Kind::PUNCTUATOR, "("}), Token({L, Kind::IDENTIFIER, "a"}),
        Token({L, Kind::OPERATOR, "+"}),   Token({L, Kind::IDENTIFIER, "b"}),
        Token({L, Kind::PUNCTUATOR, ")"}), Token({L, Kind::OPERATOR, "*"}),
        Token({L, Kind::IDENTIFIER, "c"}),
    };

    REQUIRE(compiler::parse(input1) ==
            BinaryOp{BinaryOp{Identifier{"a"}, "+", Identifier{"b"}}, "*",
                     Identifier{"c"}});
  }
}

// TEST_CASE("Parser invalid input", "[parser]") {
//   SECTION("Empty input") { SKIP("UNIMPL"); }
// }
