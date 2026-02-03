#define CATCH_CONFIG_MAIN

#include <catch2/catch_test_macros.hpp>
#include <compiler.h>
#include <datatypes.h>
#include <string>
#include <vector>

using namespace std;

const Loc L{SIZE_T_MAX, SIZE_T_MAX};

TEST_CASE("Tokenizer tests", "[tokenizer]") {
  string input1 = "if 3\nwhile \n \n else";
  string input2 = "1+2-/<>\n<=>=!====*";
  string input3 = "()), {{}}, ; ";
  string input4 = "s# 404\n2//testing\n3";

  vector<Token> output1{
      Token({L, Kind::CONDITIONAL, "if"}),
      Token({L, Kind::INT_LITERAL, "3"}),
      Token({L, Kind::IDENTIFIER, "while"}),
      Token({L, Kind::CONDITIONAL, "else"}),
  };

  vector<Token> output2{
      Token({L, Kind::INT_LITERAL, "1"}), Token({L, Kind::OPERATOR, "+"}),
      Token({L, Kind::INT_LITERAL, "2"}), Token({L, Kind::OPERATOR, "-"}),
      Token({L, Kind::OPERATOR, "/"}),    Token({L, Kind::OPERATOR, "<"}),
      Token({L, Kind::OPERATOR, ">"}),    Token({L, Kind::OPERATOR, "<="}),
      Token({L, Kind::OPERATOR, ">="}),   Token({L, Kind::OPERATOR, "!="}),
      Token({L, Kind::OPERATOR, "=="}),   Token({L, Kind::OPERATOR, "="}),
      Token({L, Kind::OPERATOR, "*"}),
  };

  vector<Token> output3{
      Token({L, Kind::PUNCTUATOR, "("}), Token({L, Kind::PUNCTUATOR, ")"}),
      Token({L, Kind::PUNCTUATOR, ")"}), Token({L, Kind::PUNCTUATOR, ","}),
      Token({L, Kind::PUNCTUATOR, "{"}), Token({L, Kind::PUNCTUATOR, "{"}),
      Token({L, Kind::PUNCTUATOR, "}"}), Token({L, Kind::PUNCTUATOR, "}"}),
      Token({L, Kind::PUNCTUATOR, ","}), Token({L, Kind::PUNCTUATOR, ";"}),
  };

  vector<Token> output4{
      Token({L, Kind::IDENTIFIER, "s"}),
      Token({L, Kind::INT_LITERAL, "2"}),
      Token({L, Kind::INT_LITERAL, "3"}),
  };

  SECTION("Basics") { REQUIRE(compiler::tokenize(input1) == output1); }
  SECTION("Operators") { REQUIRE(compiler::tokenize(input2) == output2); }
  SECTION("Punctuation") { REQUIRE(compiler::tokenize(input3) == output3); }
  SECTION("Comment") { REQUIRE(compiler::tokenize(input4) == output4); }
}
