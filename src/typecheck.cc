#include <iostream>
#include <print>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

#include "datatypes.h"
#include "expression.h"
#include "symtab.h"
#include "typecheck.h"

using namespace std;

namespace compiler {

// TODO: Move expr methods elsewhere
// ======>
C_type Literal::accept(TypeChecker &tc) const { return tc.visit(*this); }
C_type Identifier::accept(TypeChecker &tc) const { return tc.visit(*this); }
C_type UnaryOp::accept(TypeChecker &tc) const { return tc.visit(*this); }
C_type BinaryOp::accept(TypeChecker &tc) const { return tc.visit(*this); }
C_type FunctionCall::accept(TypeChecker &tc) const { return tc.visit(*this); }
C_type IfThenStatement::accept(TypeChecker &tc) const {
  return tc.visit(*this);
}
C_type IfThenElseStatement::accept(TypeChecker &tc) const {
  return tc.visit(*this);
}
C_type Block::accept(TypeChecker &tc) const { return tc.visit(*this); }
C_type Variable::accept(TypeChecker &tc) const { return tc.visit(*this); }
C_type While::accept(TypeChecker &tc) const { return tc.visit(*this); }
// <======

C_type TypeChecker::visit(const Literal &e) {
  return std::visit(
      [](const auto &arg) {
        using T = decay_t<decltype(arg)>;
        if constexpr (is_same_v<T, unsigned long>)
          return C_type::C_int;
        else if constexpr (is_same_v<T, bool>)
          return C_type::C_bool;
        else if constexpr (is_same_v<T, monostate>)
          return C_type::C_unit;
        else
          throw runtime_error("Unexpected literal");
      },
      e.value);
}

C_type TypeChecker::visit(const BinaryOp &e) {
  const auto lhs_t = e.lhs->accept(*this);
  const auto rhs_t = e.rhs->accept(*this);

  const auto sym_tab_typing = get_if<FnType>(this->sym_tab.lookup(e.op));

  if (sym_tab_typing) {
    if (lhs_t == sym_tab_typing->at(0) && rhs_t == sym_tab_typing->at(1))
      return sym_tab_typing->at(2);

    throw runtime_error("Typecheck: Invalid args: BinaryOp");
  }

  if ((e.op == "==" || e.op == "!=") && lhs_t == rhs_t)
    return C_type::C_bool;
  else if ((e.op == "=") && lhs_t == rhs_t)
    return lhs_t;
  else
    throw runtime_error("Typecheck: mismatched types on equality check");

  throw runtime_error("Typecheck: Unimplemented BinaryOp");
}

C_type TypeChecker::visit(const UnaryOp &e) {
  const auto ct = e.expr->accept(*this);

  const auto typing = get_if<FnType>(this->sym_tab.lookup(
      std::format("{}{}", e.op == "not" || e.op == "-" ? "unary_" : "", e.op)));

  if (typing) {
    if (ct == typing->at(0))
      return typing->at(1);
  }
  throw runtime_error("Unexpected arg: UnaryOp");
}

C_type TypeChecker::visit(const IfThenStatement &e) {
  const auto cond = e.condition->accept(*this);

  if (cond != C_type::C_bool)
    throw runtime_error("Condition must resolve to a boolean");

  return C_type::C_unit;
}

C_type TypeChecker::visit(const IfThenElseStatement &e) {
  const auto cond_t = e.condition->accept(*this);
  const auto then_t = e.then_branch->accept(*this);
  const auto else_t = e.else_branch->accept(*this);

  if (cond_t != C_type::C_bool)
    throw runtime_error("Condition must resolve to a boolean");

  if (then_t != else_t)
    throw runtime_error("Conditional branches must resolve to a same type");

  return then_t;
}

C_type TypeChecker::visit(const Block &e) {
  this->sym_tab.add_scope();

  if (e.exprs.empty())
    return C_type::C_unit;

  for (size_t i = 0; i < e.exprs.size() - 1; ++i)
    e.exprs[i]->accept(*this);

  auto block_t = e.exprs.back()->accept(*this);

  this->sym_tab.remove_scope();
  return block_t;
}

C_type TypeChecker::visit(const While &e) {
  auto cond = e.cond->accept(*this);

  if (cond != C_type::C_bool)
    throw runtime_error("Condition must resolve to a boolean");

  e.body->accept(*this);

  return C_type::C_unit;
}

C_type TypeChecker::visit(const Variable &e) {
  auto name = e.name;

  if (this->sym_tab.local_key_exists(name))
    throw runtime_error("Attempted to re-declare a local variable");

  auto var_type = e.value->accept(*this);

  std::println("# Typecheck: var {}, type {}", name,
               static_cast<int>(var_type));

  this->sym_tab.add(name, var_type);

  for (auto x : this->sym_tab.stack.back()) {
    auto tmp = get_if<C_type>(&x.second);
    if (tmp) {
      // cout << x.first << static_cast<int>(*tmp) << "\n";
    }
  }

  // println("Assigned var, returning now");

  return C_type::C_unit;
}

C_type TypeChecker::visit(const Identifier &e) {
  auto symEntry = this->sym_tab.lookup(e.value);

  if (!symEntry)
    throw runtime_error("Variable not declared or unidentified identifier");

  if (symEntry->index() == 0) {
    auto tmp = get<C_type>(*symEntry);
    // println("Fetched and will now return {}", static_cast<int>(tmp));
    return tmp;
  }

  return get<FnType>(*symEntry).back();
}

C_type TypeChecker::visit(const FunctionCall &e) {
  auto symEntry = this->sym_tab.lookup(e.name->value);

  if (!symEntry)
    throw runtime_error("Function with given name was not found");

  if (symEntry->index() == 0)
    throw runtime_error("Variable did not contain a function call");

  auto expected = get<FnType>(*symEntry);

  if (expected.size() - 1 != e.args.size())
    throw runtime_error("Function received incorrect amount of arguments");

  for (size_t i = 0; i < e.args.size(); ++i)
    if (e.args[i]->accept(*this) != expected[i])
      throw runtime_error("Non-matching function "
                          "argument");

  return expected.back();
}

C_type typecheck(UPtrExpr &&root) {
  TypeChecker tc{};

  return root->accept(tc);
}

} // namespace compiler
