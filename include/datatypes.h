#pragma once

#include <cstddef>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

enum class Kind {
  IDENTIFIER,
  INT_LITERAL,
  OPERATOR,
  PUNCTUATOR,
  COMMENT,
  CONDITIONAL,
  END,
};


struct Loc {
  size_t row;
  size_t col;

  bool operator==(const Loc &other) const {
    return (row == other.row && col == other.col) ||
           (other.row == SIZE_MAX && other.col == SIZE_MAX);
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
