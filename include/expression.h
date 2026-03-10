#pragma once

#include <algorithm>
#include <memory>
#include <ostream>
#include <variant>
#include <vector>

#include "datatypes.h"
#include "ir.h"

namespace compiler {

struct TypeChecker;

// TODO: Move logic out of header

struct Expression {
  virtual ~Expression() = default;
  virtual bool equals(const Expression &other) const = 0;
  virtual void print(std::ostream &os) const = 0;

  virtual C_type accept(TypeChecker &tc) const = 0;
  virtual IRVar accept(IRGenerator &tc) const = 0;
};

using UPtrExpr = std::unique_ptr<Expression>;

struct Literal : Expression {
  std::variant<int, bool, std::monostate> value;

  explicit Literal(int i) : value(i) {};
  explicit Literal(bool x) : value(x) {};
  explicit Literal(std::monostate u) : value(u) {};

  C_type accept(TypeChecker &tc) const override;
  IRVar accept(IRGenerator &ir) const override;

  bool equals(const Expression &other) const override {
    if (auto *o = dynamic_cast<const Literal *>(&other)) {
      return value == o->value;
    };
    return false;
  };

  void print(std::ostream &os) const override {
    os << "Literal ";
    std::visit(
        [&os](const auto &v) {
          using T = std::decay_t<decltype(v)>;
          if constexpr (std::is_same_v<T, std::monostate>) {
            os << "{}";
          } else {
            os << v;
          }
        },
        value);
  }
};

struct Identifier : Expression {
  std::string value;

  explicit Identifier(std::string x) : value(std::move(x)) {};

  C_type accept(TypeChecker &tc) const override;
  IRVar accept(IRGenerator &ir) const override;

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
  std::vector<UPtrExpr> args;

  explicit FunctionCall(std::unique_ptr<Identifier> x, std::vector<UPtrExpr> a)
      : name(std::move(x)), args(std::move(a)) {};

  C_type accept(TypeChecker &tc) const override;
  IRVar accept(IRGenerator &ir) const override;

  bool equals(const Expression &other) const override {
    if (auto *o = dynamic_cast<const FunctionCall *>(&other)) {
      auto eq = [](const UPtrExpr &a, const UPtrExpr &b) {
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

struct UnaryOp : Expression {
  std::string op;
  UPtrExpr expr;

  explicit UnaryOp(std::string o, UPtrExpr e) : op(o), expr(std::move(e)) {};

  C_type accept(TypeChecker &tc) const override;
  IRVar accept(IRGenerator &ir) const override;

  bool equals(const Expression &other) const override {
    if (auto *o = dynamic_cast<const UnaryOp *>(&other)) {
      return op == o->op && expr->equals(*o->expr);
    };
    return false;
  };

  void print(std::ostream &os) const override {
    os << "UnaryOp(" << op << " ";
    expr->print(os);
    os << ")";
  }
};

struct BinaryOp : Expression {
  UPtrExpr lhs;
  std::string op;
  UPtrExpr rhs;

  explicit BinaryOp(UPtrExpr l, std::string o, UPtrExpr r)
      : lhs(std::move(l)), op(o), rhs(std::move(r)) {};

  C_type accept(TypeChecker &tc) const override;
  IRVar accept(IRGenerator &ir) const override;

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
  UPtrExpr condition;
  UPtrExpr then_branch;

  explicit IfThenStatement(UPtrExpr c, UPtrExpr t)
      : condition(std::move(c)), then_branch(std::move(t)) {};

  C_type accept(TypeChecker &tc) const override;
  IRVar accept(IRGenerator &ir) const override;

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
  UPtrExpr condition;
  UPtrExpr then_branch;
  UPtrExpr else_branch;

  explicit IfThenElseStatement(UPtrExpr c, UPtrExpr t, UPtrExpr e)
      : condition(std::move(c)), then_branch(std::move(t)),
        else_branch(std::move(e)) {}

  C_type accept(TypeChecker &tc) const override;
  IRVar accept(IRGenerator &ir) const override;

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

struct Block : Expression {
  std::vector<UPtrExpr> exprs;

  explicit Block(std::vector<UPtrExpr> e) : exprs(std::move(e)) {};

  C_type accept(TypeChecker &tc) const override;
  IRVar accept(IRGenerator &ir) const override;

  bool equals(const Expression &other) const override {
    if (auto *o = dynamic_cast<const Block *>(&other)) {
      auto eq = [](const UPtrExpr &a, const UPtrExpr &b) {
        return a->equals(*b);
      };
      if (exprs.size() == o->exprs.size() &&
          std::equal(exprs.begin(), exprs.end(), o->exprs.begin(), eq))
        return true;
    };
    return false;
  };

  void print(std::ostream &os) const override {
    os << "Block: {\n";
    for (size_t i = 0; i < exprs.size(); ++i) {
      os << "  ";
      exprs[i]->print(os);
      os << '\n';
    }
    os << "}";
  }
};

struct Variable : Expression {
  std::string name;
  UPtrExpr value;

  explicit Variable(std::string n, UPtrExpr v)
      : name(std::move(n)), value(std::move(v)) {};

  C_type accept(TypeChecker &tc) const override;
  IRVar accept(IRGenerator &ir) const override;

  bool equals(const Expression &other) const override {
    if (auto *o = dynamic_cast<const Variable *>(&other)) {
      return name == o->name && value->equals(*o->value);
    }
    return false;
  }

  void print(std::ostream &os) const override {
    os << "Var " << name << " = (";
    value->print(os);
    os << ") ";
  }
};

struct While : Expression {
  UPtrExpr cond;
  UPtrExpr body;

  explicit While(UPtrExpr c, UPtrExpr b)
      : cond(std::move(c)), body(std::move(b)) {};

  C_type accept(TypeChecker &tc) const override;
  IRVar accept(IRGenerator &ir) const override;

  bool equals(const Expression &other) const override {
    if (auto *o = dynamic_cast<const While *>(&other)) {
      return cond->equals(*o->cond) && body->equals(*o->body);
    };
    return false;
  };

  void print(std::ostream &os) const override {
    os << "While ";
    cond->print(os);
    os << " do ";
    body->print(os);
  }
};

inline bool operator==(const Expression &a, const Expression &b) {
  return a.equals(b);
}

inline std::ostream &operator<<(std::ostream &os, const Expression &e) {
  e.print(os);
  return os;
}

} // namespace compiler
