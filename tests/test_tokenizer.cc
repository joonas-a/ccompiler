#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#endif

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

#include "compiler.h"
#include "datatypes.h"

using namespace std;

const Loc L{SIZE_MAX, SIZE_MAX};

TEST_CASE("Tokenizer tests", "[tokenizer]") {
  string input1 = "if 3\nwhile \n \n else";
  string input2 = "1+2-/<>\n<=>=!====* and a or not 2";
  string input3 = "()), {{}}, ; ";
  string input4 = "s# 404\n2//testing\n3";
  string input5 = "(a+b)*c";
  string input6 = "a b+c";
  string input7 = "{{a}{b}}";

  vector<Token> output1{
      Token{L, Kind::PUNCTUATOR, "{"},    Token{L, Kind::CONDITIONAL, "if"},
      Token{L, Kind::INT_LITERAL, "3"},    Token{L, Kind::IDENTIFIER, "while"},
      Token{L, Kind::CONDITIONAL, "else"}, Token{L, Kind::PUNCTUATOR, "}"},
      Token{L, Kind::END, "$SENTINEL"}};

  vector<Token> output2{
      Token{L, Kind::PUNCTUATOR, "{"}, Token{L, Kind::INT_LITERAL, "1"},
      Token{L, Kind::OPERATOR, "+"},    Token{L, Kind::INT_LITERAL, "2"},
      Token{L, Kind::OPERATOR, "-"},    Token{L, Kind::OPERATOR, "/"},
      Token{L, Kind::OPERATOR, "<"},    Token{L, Kind::OPERATOR, ">"},
      Token{L, Kind::OPERATOR, "<="},   Token{L, Kind::OPERATOR, ">="},
      Token{L, Kind::OPERATOR, "!="},   Token{L, Kind::OPERATOR, "=="},
      Token{L, Kind::OPERATOR, "="},    Token{L, Kind::OPERATOR, "*"},
      Token{L, Kind::OPERATOR, "and"},  Token{L, Kind::IDENTIFIER, "a"},
      Token{L, Kind::OPERATOR, "or"},   Token{L, Kind::OPERATOR, "not"},
      Token{L, Kind::INT_LITERAL, "2"}, Token{L, Kind::PUNCTUATOR, "}"},
      Token{L, Kind::END, "$SENTINEL"}};

  vector<Token> output3{
      Token{L, Kind::PUNCTUATOR, "{"}, Token{L, Kind::PUNCTUATOR, "("},
      Token{L, Kind::PUNCTUATOR, ")"},  Token{L, Kind::PUNCTUATOR, ")"},
      Token{L, Kind::PUNCTUATOR, ","},  Token{L, Kind::PUNCTUATOR, "{"},
      Token{L, Kind::PUNCTUATOR, "{"},  Token{L, Kind::PUNCTUATOR, "}"},
      Token{L, Kind::PUNCTUATOR, "}"},  Token{L, Kind::PUNCTUATOR, ","},
      Token{L, Kind::PUNCTUATOR, ";"},  Token{L, Kind::PUNCTUATOR, "}"},
      Token{L, Kind::END, "$SENTINEL"}};

  vector<Token> output4{
      Token{L, Kind::PUNCTUATOR, "{"}, Token{L, Kind::IDENTIFIER, "s"},
      Token{L, Kind::INT_LITERAL, "2"}, Token{L, Kind::INT_LITERAL, "3"},
      Token{L, Kind::PUNCTUATOR, "}"}, Token{L, Kind::END, "$SENTINEL"}};

  vector<Token> output5{
      Token{L, Kind::PUNCTUATOR, "{"}, Token{L, Kind::PUNCTUATOR, "("},
      Token{L, Kind::IDENTIFIER, "a"},  Token{L, Kind::OPERATOR, "+"},
      Token{L, Kind::IDENTIFIER, "b"},  Token{L, Kind::PUNCTUATOR, ")"},
      Token{L, Kind::OPERATOR, "*"},    Token{L, Kind::IDENTIFIER, "c"},
      Token{L, Kind::PUNCTUATOR, "}"}, Token{L, Kind::END, "$SENTINEL"}};

  vector<Token> output6{
      Token{L, Kind::PUNCTUATOR, "{"}, Token{L, Kind::IDENTIFIER, "a"},
      Token{L, Kind::IDENTIFIER, "b"},  Token{L, Kind::OPERATOR, "+"},
      Token{L, Kind::IDENTIFIER, "c"},  Token{L, Kind::PUNCTUATOR, "}"},
      Token{L, Kind::END, "$SENTINEL"}};

  vector<Token> output7{
      Token{L, Kind::PUNCTUATOR, "{"}, Token{L, Kind::PUNCTUATOR, "{"},
      Token{L, Kind::PUNCTUATOR, "{"},  Token{L, Kind::IDENTIFIER, "a"},
      Token{L, Kind::PUNCTUATOR, "}"},  Token{L, Kind::PUNCTUATOR, "{"},
      Token{L, Kind::IDENTIFIER, "b"},  Token{L, Kind::PUNCTUATOR, "}"},
      Token{L, Kind::PUNCTUATOR, "}"},  Token{L, Kind::PUNCTUATOR, "}"},
      Token{L, Kind::END, "$SENTINEL"}};

  SECTION("Basics") { REQUIRE(compiler::tokenize(input1) == output1); }
  SECTION("Operators") { REQUIRE(compiler::tokenize(input2) == output2); }
  SECTION("Punctuation") { REQUIRE(compiler::tokenize(input3) == output3); }
  SECTION("Comment") { REQUIRE(compiler::tokenize(input4) == output4); }
  SECTION("Valid binary op") { REQUIRE(compiler::tokenize(input5) == output5); }
  SECTION("Invalid syntax") { REQUIRE(compiler::tokenize(input6) == output6); }
  SECTION("Block test") { REQUIRE(compiler::tokenize(input7) == output7); }
}
