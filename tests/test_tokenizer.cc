#define CATCH_CONFIG_MAIN

#include <catch2/catch_test_macros.hpp>
#include <compiler.h>
#include <datatypes.h>
#include <string>
#include <vector>

using namespace std;

const Loc L{SIZE_T_MAX, SIZE_T_MAX};

TEST_CASE("Tokenizer basic tests", "[tokenizer]") {
  string input1 = "if 3\nwhile \n \n else";
  vector<Token> output1{
      Token({L, Kind::CONDITIONAL, "if"}),
      Token({L, Kind::INT_LITERAL, "3"}),
      Token({L, Kind::IDENTIFIER, "while"}),
      Token({L, Kind::CONDITIONAL, "else"}),
  };

  SECTION("it works") { REQUIRE(compiler::tokenize(input1) == output1); }
}
