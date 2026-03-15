#include <print>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

#include "expression.h"
#include "ir.h"

// IRGen assumes a thorough typechecking has been done and
// existence of any identifiers / other symbols confirmed

// TODOS:
// Symbol table and scopes

namespace compiler {

IRVar Literal::accept(IRGenerator &tc) const { return tc.visit(*this); }
IRVar Identifier::accept(IRGenerator &tc) const { return tc.visit(*this); }
IRVar UnaryOp::accept(IRGenerator &tc) const { return tc.visit(*this); }
IRVar BinaryOp::accept(IRGenerator &tc) const { return tc.visit(*this); }
IRVar FunctionCall::accept(IRGenerator &tc) const { return tc.visit(*this); }
IRVar IfThenStatement::accept(IRGenerator &tc) const { return tc.visit(*this); }
IRVar IfThenElseStatement::accept(IRGenerator &tc) const {
  return tc.visit(*this);
}
IRVar Block::accept(IRGenerator &tc) const { return tc.visit(*this); }
IRVar Variable::accept(IRGenerator &tc) const { return tc.visit(*this); }
IRVar While::accept(IRGenerator &tc) const { return tc.visit(*this); }

IRVar IRGenerator::visit(const Literal &e) {
  return std::visit(
      [this](const auto &arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, std::monostate>) {
          return this->utils.generate_unit();
        }

        const auto var = this->utils.generate_var();

        if constexpr (std::is_same_v<T, unsigned long>) {
          this->ins.emplace_back(LoadIntConst{arg, var});
          return var;
        }

        if constexpr (std::is_same_v<T, bool>) {
          this->ins.emplace_back(LoadBoolConst{arg, var});
          return var;
        }
        throw std::runtime_error("Unknown literal");
      },
      e.value);
}

IRVar IRGenerator::visit(const BinaryOp &e) {
  const auto lhs_t = e.lhs->accept(*this);

  if (e.op == "or" || e.op == "and") {
    const auto is_or = e.op == "or";
    const auto [rhs_label, skip_label, end_label] =
        this->utils.generate_labels(is_or ? "or" : "and");

    this->ins.emplace_back(CondJump{lhs_t, is_or ? skip_label : rhs_label,
                                    is_or ? rhs_label : skip_label});

    this->ins.emplace_back(rhs_label);
    const auto rhs_t = e.rhs->accept(*this);
    const auto dst = this->utils.generate_var();
    this->ins.emplace_back(Copy{rhs_t, dst});
    this->ins.emplace_back(Jump{end_label});

    this->ins.emplace_back(skip_label);
    this->ins.emplace_back(LoadBoolConst{is_or ? true : false, dst});
    this->ins.emplace_back(Jump{end_label});

    this->ins.emplace_back(end_label);
    return dst;
  }

  if (e.op == "=") {
    const auto rhs_t = e.rhs->accept(*this);
    this->ins.emplace_back(Copy{rhs_t, lhs_t});
    return lhs_t;
  }

  const auto rhs_t = e.rhs->accept(*this);

  std::vector<IRVar> args{lhs_t, rhs_t};
  const auto dst = this->utils.generate_var();

  this->ins.emplace_back(Call{e.op, args, dst});
  return dst;
}

IRVar IRGenerator::visit(const UnaryOp &e) {
  const auto args = std::vector<IRVar>{e.expr->accept(*this)};
  const auto dst = this->utils.generate_var();

  this->ins.emplace_back(Call{std::format("unary_{}", e.op), args, dst});
  return dst;
}

IRVar IRGenerator::visit(const IfThenStatement &e) {
  const auto [then_label, end_label, _] = this->utils.generate_labels("ifthen");

  const auto cond = e.condition->accept(*this);
  this->ins.emplace_back(CondJump{cond, then_label, end_label});

  this->ins.emplace_back(then_label);

  e.then_branch->accept(*this);

  this->ins.emplace_back(end_label);

  return this->utils.generate_unit();
}

IRVar IRGenerator::visit(const IfThenElseStatement &e) {
  const auto [then_label, end_label, else_label] =
      this->utils.generate_labels("ifthenelse");

  auto return_var = this->utils.generate_var();

  const auto cond = e.condition->accept(*this);

  this->ins.emplace_back(CondJump{cond, then_label, else_label});

  this->ins.emplace_back(then_label);

  const auto then_var = e.then_branch->accept(*this);
  this->ins.emplace_back(Copy{then_var, return_var});
  this->ins.emplace_back(Jump{end_label});

  this->ins.emplace_back(else_label);
  const auto else_var = e.else_branch->accept(*this);
  this->ins.emplace_back(Copy{else_var, return_var});

  this->ins.emplace_back(end_label);

  return return_var;
}

IRVar IRGenerator::visit(const Block &e) {
  if (e.exprs.empty())
    return this->utils.generate_unit();

  this->sym_tab.add_scope();

  for (size_t i = 0; i < e.exprs.size() - 1; ++i) {
    e.exprs[i]->accept(*this);
  }

  auto block_var = e.exprs.back()->accept(*this);

  this->sym_tab.remove_scope();
  return block_var;
}

IRVar IRGenerator::visit(const While &e) {
  const auto [start_label, body_label, end_label] =
      this->utils.generate_labels("while");

  this->ins.emplace_back(start_label);
  const auto cond_var = e.cond->accept(*this);

  this->ins.emplace_back(CondJump{cond_var, body_label, end_label});

  this->ins.emplace_back(body_label);
  e.body->accept(*this);
  this->ins.emplace_back(Jump{start_label});

  this->ins.emplace_back(end_label);

  return this->utils.generate_unit();
}

IRVar IRGenerator::visit(const Variable &e) {
  const auto rhs = e.value->accept(*this);
  const auto lhs = this->utils.generate_var();

  std::println("# Debug: IRGen: Copy({}, {})", rhs, lhs);
  // Naming clashes handled in typechecker
  this->sym_tab.add(e.name, lhs);
  this->ins.emplace_back(Copy{rhs, lhs});

  return this->utils.generate_unit();
}

IRVar IRGenerator::visit(const Identifier &e) {
  return *this->sym_tab.lookup(e.value);
}

IRVar IRGenerator::visit(const FunctionCall &e) {
  const auto fn = this->sym_tab.lookup(e.name->value);

  std::println("# IR FunctionCall: {}", *fn);

  auto args = std::vector<IRVar>{};
  for (auto &arg : e.args)
    args.emplace_back(arg->accept(*this));

  const auto dst = this->utils.generate_var();
  this->ins.emplace_back(Call{*fn, args, dst});

  std::println("# IR: Fn call return");

  return dst;
}

IRVar IRUtils::generate_var() {
  ++var_count;

  const auto new_var = std::format("x{}", var_count);
  this->ir_vars.push_back(new_var);

  return new_var;
}

IRVar IRUtils::generate_unit() {
  static constexpr IRVar unit = "unit";
  if (!this->unit_dispatched) {
    this->ir_vars.push_back(unit);
    this->unit_dispatched = true;
  }
  return unit;
}

Labels IRUtils::generate_labels(std::string_view keyword) {
  ++label_count;
  return std::make_tuple(std::format("main_{}_right{}", keyword, label_count),
                         std::format("main_{}_skip{}", keyword, label_count),
                         std::format("main_{}_end{}", keyword, label_count));
}

// TODO: add types to ast nodes
IRGenerator generate_ir(UPtrExpr &root, C_type root_type) {
  IRGenerator ir_gen{};

  auto root_ir_var = root->accept(ir_gen);

  if (!std::holds_alternative<C_unit>(root_type)) {
    auto dst = ir_gen.utils.generate_var();
    auto args = std::vector<IRVar>{root_ir_var};

    if (std::holds_alternative<C_int>(root_type))
      ir_gen.ins.emplace_back(Call{"print_int", args, dst});

    if (std::holds_alternative<C_bool>(root_type))
      ir_gen.ins.emplace_back(Call{"print_bool", args, dst});
  }

  return ir_gen;
}

} // namespace compiler
