#pragma once

#include <cstddef>
#include <datatypes.h>
#include <string>

enum class Kind {
  IDENTIFIER,
  INT_LITERAL,
  OPERATOR,
  PUNCTUATOR,
  COMMENT,
  MULTI_COMMENT_START,
  MULTI_COMMENT_END,
  CONDITIONAL,
};

struct Loc {
  size_t row;
  size_t col;

  bool operator==(const Loc &other) const {
    return (row == other.row && col == other.col) ||
           (other.row == SIZE_T_MAX && other.col == SIZE_T_MAX);
  }
};

struct Token {
  Loc loc;
  Kind type;
  std::string text;

  bool operator==(const Token &) const = default;
};
