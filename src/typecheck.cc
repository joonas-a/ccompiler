#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

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
      [](const auto &arg) -> C_type {
        using T = decay_t<decltype(arg)>;
        if constexpr (is_same_v<T, unsigned long>)
          return C_int{};
        else if constexpr (is_same_v<T, bool>)
          return C_bool{};
        else if constexpr (is_same_v<T, monostate>)
          return C_unit{};
        else
          throw runtime_error("Unexpected literal");
      },
      e.value);
}

C_type TypeChecker::visit(const BinaryOp &e) {
  const auto lhs_t = e.lhs->accept(*this);
  const auto rhs_t = e.rhs->accept(*this);

  const auto typing = get_fn(*this->sym_tab.lookup(e.op));

  if (typing) {
    if (lhs_t.index() == typing->get()->args.at(0).index() &&
        rhs_t.index() == typing->get()->args.at(1).index())
      return typing->get()->return_type;

    throw runtime_error("Typecheck: Invalid args: BinaryOp");
  }

  if ((e.op == "==" || e.op == "!=") && lhs_t.index() == rhs_t.index())
    return C_bool{};
  else if ((e.op == "=") && lhs_t.index() == rhs_t.index())
    return lhs_t;
  else
    throw runtime_error("Typecheck: mismatched types on equality check");

  throw runtime_error("Typecheck: Unimplemented BinaryOp");
}

C_type TypeChecker::visit(const UnaryOp &e) {
  const auto ct = e.expr->accept(*this);

  const auto typing = get_fn(*this->sym_tab.lookup(
      std::format("{}{}", e.op == "not" || e.op == "-" ? "unary_" : "", e.op)));

  if (typing) {
    if (ct.index() == typing->get()->args.at(0).index())
      return typing->get()->return_type;
  }
  throw runtime_error("Typecheck: Unexpected arg: UnaryOp");
}

C_type TypeChecker::visit(const IfThenStatement &e) {
  const auto cond = e.condition->accept(*this);

  if (!std::get_if<C_bool>(&cond))
    throw runtime_error("Typecheck: Condition must resolve to a boolean");

  return C_unit{};
}

C_type TypeChecker::visit(const IfThenElseStatement &e) {
  const auto cond_t = e.condition->accept(*this);
  const auto then_t = e.then_branch->accept(*this);
  const auto else_t = e.else_branch->accept(*this);

  if (!std::get_if<C_bool>(&cond_t))
    throw runtime_error("Typecheck: Condition must resolve to a boolean");

  if (then_t.index() != else_t.index())
    throw runtime_error(
        "Typecheck: Conditional branches must resolve to a same type");

  return then_t;
}

C_type TypeChecker::visit(const Block &e) {
  this->sym_tab.add_scope();

  if (e.exprs.empty())
    return C_unit{};

  for (size_t i = 0; i < e.exprs.size() - 1; ++i)
    e.exprs[i]->accept(*this);

  auto block_t = e.exprs.back()->accept(*this);

  this->sym_tab.remove_scope();
  return block_t;
}

C_type TypeChecker::visit(const While &e) {
  auto cond = e.cond->accept(*this);

  if (!std::get_if<C_bool>(&cond))
    throw runtime_error("Typecheck: Condition must resolve to a boolean");

  e.body->accept(*this);

  return C_unit{};
}

C_type TypeChecker::visit(const Variable &e) {
  auto name = e.name;

  if (this->sym_tab.local_key_exists(name))
    throw runtime_error("Typecheck: Attempted to re-declare a local variable");

  auto var_type = e.value->accept(*this);

  this->sym_tab.add(name, var_type);

  return C_unit{};
}

C_type TypeChecker::visit(const Identifier &e) {
  auto typing = this->sym_tab.lookup(e.value);

  if (!typing)
    throw runtime_error(
        "Typecheck: Variable not declared or unidentified identifier");

  return *typing;
}

C_type TypeChecker::visit(const FunctionCall &e) {
  auto typing = this->sym_tab.lookup(e.name->value);

  if (!typing)
    throw runtime_error("Typecheck: Function with given name was not found");

  auto fn_typing = get_fn(*typing);

  if (!fn_typing) {
    throw runtime_error(
        std::format("Typecheck: Variable {} did not contain a function call",
                    e.name->value));
  }

  if (fn_typing->get()->args.size() != e.args.size())
    throw runtime_error("Function received incorrect amount of arguments");

  for (size_t i = 0; i < e.args.size(); ++i)
    if (e.args[i]->accept(*this).index() !=
        fn_typing->get()->args.at(i).index())
      throw runtime_error("Typecheck: Non-matching function "
                          "argument");

  return fn_typing->get()->return_type;
}

C_type typecheck(UPtrExpr &&root) {
  TypeChecker tc{};

  return root->accept(tc);
}

} // namespace compiler
