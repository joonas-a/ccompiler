#pragma once

#include <array>
#include <cstddef>
#include <datatypes.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>

// === Tokenizer types ===
enum class Kind {
  IDENTIFIER,
  INT_LITERAL,
  OPERATOR,
  PUNCTUATOR,
  COMMENT,
  CONDITIONAL,
  END,
};

static constexpr std::array<std::string_view, 15> OPERATORS = {
    "+",  "-", "*", "/", "=",   "==", "!=", "<=",
    ">=", "<", ">", "%", "and", "or", "not"};
static constexpr std::array<std::string_view, 6> PUNCTUATORS = {"(", ")", "{",
                                                                "}", ",", ";"};
static constexpr std::array<std::string_view, 2> COMMENTS = {"//", "#"};
static constexpr std::array<std::string_view, 3> CONDITIONALS = {"if", "then",
                                                                 "else"};

struct Loc {
  size_t row;
  size_t col;

  bool operator==(const Loc &other) const {
    return (row == other.row && col == other.col) ||
           (other.row == SIZE_T_MAX && other.col == SIZE_T_MAX);
  }

  friend std::ostream &operator<<(std::ostream &out, const Loc &loc) {
    return out << "Row: " << loc.row << ", Col: " << loc.col;
  }
};

struct Token {
  Loc loc;
  Kind type;
  std::string text;

  bool operator==(const Token &) const = default;

  std::string as_string() const {
    std::ostringstream oss;
    oss << this->text << ", " << this->loc;
    return oss.str();
  }
};
