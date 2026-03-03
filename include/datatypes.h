#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <datatypes.h>
#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

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

static constexpr std::array<std::string_view, 12> OPERATORS = {
    "+", "-", "*", "/", "=", "==", "!=", "<=", ">=", "<", ">", "%"};
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

// === Parser types ===

struct Expression {
  virtual ~Expression() = default;
  virtual bool equals(const Expression &other) const = 0;
  virtual void print(std::ostream &os) const = 0;
};

struct Literal : Expression {
  std::variant<int, bool> value;

  explicit Literal(int i) : value(i) {};
  explicit Literal(bool x) : value(x) {};

  bool equals(const Expression &other) const override {
    if (auto *o = dynamic_cast<const Literal *>(&other)) {
      return value == o->value;
    };
    return false;
  };

  void print(std::ostream &os) const override {
    os << "Literal ";
    std::visit([&os](const auto &v) { os << v; }, value);
  }
};

struct Identifier : Expression {
  std::string value;

  explicit Identifier(std::string x) : value(std::move(x)) {};

  bool equals(const Expression &other) const override {
    if (auto *o = dynamic_cast<const Identifier *>(&other)) {
      return value == o->value;
    };
    return false;
  };

  void print(std::ostream &os) const override { os << "\"" << value << "\""; }
};

struct FunctionCall : Expression {
  std::unique_ptr<Identifier> name;
  std::vector<std::unique_ptr<Expression>> args;

  explicit FunctionCall(std::unique_ptr<Identifier> x,
                        std::vector<std::unique_ptr<Expression>> a)
      : name(std::move(x)), args(std::move(a)) {};

  bool equals(const Expression &other) const override {
    if (auto *o = dynamic_cast<const FunctionCall *>(&other)) {
      auto eq = [](const std::unique_ptr<Expression> &a,
                   const std::unique_ptr<Expression> &b) {
        return a->equals(*b);
      };
      if (name->equals(*o->name) && args.size() == o->args.size() &&
          std::equal(args.begin(), args.end(), o->args.begin(), eq))
        return true;
    };
    return false;
  };

  void print(std::ostream &os) const override {
    os << "Function (";
    name->print(os);
    os << ", Args: (";
    for (size_t i = 0; i < args.size(); ++i) {
      args[i]->print(os);
      if (i + 1 < args.size())
        os << ", ";
    }
    os << "))";
  }
};

struct BinaryOp : Expression {
  std::unique_ptr<Expression> lhs;
  std::string op;
  std::unique_ptr<Expression> rhs;

  explicit BinaryOp(std::unique_ptr<Expression> l, std::string o,
                    std::unique_ptr<Expression> r)
      : lhs(std::move(l)), op(o), rhs(std::move(r)) {};

  bool equals(const Expression &other) const override {
    if (auto *o = dynamic_cast<const BinaryOp *>(&other)) {
      return lhs->equals(*o->lhs) && op == o->op && rhs->equals(*o->rhs);
    };
    return false;
  };

  void print(std::ostream &os) const override {
    os << "BinaryOp(";
    lhs->print(os);
    os << " " << op << " ";
    rhs->print(os);
    os << ")";
  }
};

struct IfThenStatement : Expression {
  std::unique_ptr<Expression> condition;
  std::unique_ptr<Expression> then_branch;

  explicit IfThenStatement(std::unique_ptr<Expression> c,
                           std::unique_ptr<Expression> t)
      : condition(std::move(c)), then_branch(std::move(t)) {};

  bool equals(const Expression &other) const override {
    if (auto *o = dynamic_cast<const IfThenStatement *>(&other)) {
      return condition->equals(*o->condition) &&
             then_branch->equals(*o->then_branch);
    };
    return false;
  };

  void print(std::ostream &os) const override {
    condition->print(os);
    os << " ? ";
    then_branch->print(os);
  }
};

struct IfThenElseStatement : Expression {
  std::unique_ptr<Expression> condition;
  std::unique_ptr<Expression> then_branch;
  std::unique_ptr<Expression> else_branch;

  explicit IfThenElseStatement(std::unique_ptr<Expression> c,
                               std::unique_ptr<Expression> t,
                               std::unique_ptr<Expression> e)
      : condition(std::move(c)), then_branch(std::move(t)),
        else_branch(std::move(e)) {}

  bool equals(const Expression &other) const override {
    if (auto *o = dynamic_cast<const IfThenElseStatement *>(&other)) {
      return condition->equals(*o->condition) &&
             then_branch->equals(*o->then_branch) &&
             else_branch->equals(*o->else_branch);
    };
    return false;
  };

  void print(std::ostream &os) const override {
    condition->print(os);
    os << " ? ";
    then_branch->print(os);
    os << " : ";
    else_branch->print(os);
  }
};

inline bool operator==(const Expression &a, const Expression &b) {
  return a.equals(b);
}

inline std::ostream &operator<<(std::ostream &os, const Expression &e) {
  e.print(os);
  return os;
}
