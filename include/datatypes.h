#pragma once

#include <cstddef>
#include <datatypes.h>
#include <string>
#include <string_view>
#include <variant>

// Tokenizer types
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
           (other.row == SIZE_T_MAX && other.col == SIZE_T_MAX);
  }
};

struct Token {
  Loc loc;
  Kind type;
  std::string text;

  bool operator==(const Token &) const = default;
};

// Parser types
struct Expression {
  bool operator==(const Expression &) const = default;
};

struct Literal : Expression {
  std::variant<int, bool> value;

  explicit Literal(int i) : value(i) {};
  explicit Literal(bool x) : value(x) {};

  bool operator==(const Literal &) const = default;
};

struct Identifier : Expression {
  std::string_view name;
};

struct BinaryOp : Expression {
  Expression left;
  std::string_view op;
  Expression right;

  explicit BinaryOp(Expression l, std::string_view o, Expression r)
      : left(std::move(l)), op(o), right(std::move(r)) {};
};
