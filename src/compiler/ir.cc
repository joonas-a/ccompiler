#include <tuple>
#include <type_traits>

#include "expression.h"
#include "ir.h"

struct SymTab;

namespace compiler {

IRGenerator::IRGenerator(IRUtils &utils) : utils(utils) {};

// TODO: Move expr methods elsewhere
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

        const auto var = this->utils.generate_var();
        if constexpr (std::is_same_v<T, int>) {
          this->ins.emplace_back(LoadIntConst{arg, var});
          return var;
        } else if constexpr (std::is_same_v<T, bool>) {
          this->ins.emplace_back(LoadBoolConst{arg, var});
          return var;
        } else if constexpr (std::is_same_v<T, std::monostate>) {
          return IRVar{"unit"};
        } else
          throw std::runtime_error("Unknown literal");
      },
      e.value);
}

IRVar IRGenerator::visit(const BinaryOp &e) {
  const auto lhs_t = e.lhs->accept(*this);
  const auto rhs_t = e.rhs->accept(*this);

  std::vector<IRVar> args{lhs_t, rhs_t};
  const auto dst = this->utils.generate_var();

  this->ins.emplace_back(Call{e.op, args, dst});
  return dst;
}

IRVar IRGenerator::visit(const UnaryOp &e) {
  const auto args = std::vector<IRVar>{e.expr->accept(*this)};
  const auto dst = this->utils.generate_var();

  this->ins.emplace_back(Call{e.op, args, dst});
  return dst;
}

IRVar IRGenerator::visit(const IfThenStatement &e) {
  const auto [then_label, end_label, _] = this->utils.generate_labels(false);

  const auto cond = e.condition->accept(*this);
  this->ins.emplace_back(CondJump{cond, then_label, end_label});

  this->ins.emplace_back(then_label);

  e.then_branch->accept(*this);

  this->ins.emplace_back(end_label);

  return "unit";
}

IRVar IRGenerator::visit(const IfThenElseStatement &e) {
  const auto [then_label, end_label, else_label] =
      this->utils.generate_labels(false);

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
    return "unit";

  for (size_t i = 0; i < e.exprs.size() - 1; ++i) {
    e.exprs[i]->accept(*this);
  }

  return e.exprs.back()->accept(*this);
}

IRVar IRGenerator::visit(const While &e) {
  const auto [start_label, body_label, end_label] =
      this->utils.generate_labels(true);

  this->ins.emplace_back(start_label);
  const auto cond_var = e.cond->accept(*this);

  this->ins.emplace_back(CondJump{cond_var, body_label, end_label});

  this->ins.emplace_back(body_label);
  e.body->accept(*this);
  this->ins.emplace_back(Jump{start_label});

  this->ins.emplace_back(end_label);

  return "unit";
}

// TODO: make scopes work
IRVar IRGenerator::visit(const Variable &e) {
  const auto rhs = e.value->accept(*this);
  const auto lhs = this->utils.generate_var();

  this->sym_tab.emplace(e.name, lhs);
  this->ins.emplace_back(Copy{rhs, lhs});

  return "unit";
}

IRVar IRGenerator::visit(const Identifier &e) {
  // Existence already verified in typechecker
  return this->sym_tab.find(e.value)->second;
}

IRVar IRGenerator::visit(const FunctionCall &e) {
  const auto fn_name = this->sym_tab.find(e.name->value)->second;

  auto args = std::vector<IRVar>{};
  for (auto &arg : e.args)
    args.emplace_back(arg->accept(*this));

  const auto dst = this->utils.generate_var();
  this->ins.emplace_back(Call{fn_name, args, dst});

  return "unit";
}

IRVar IRUtils::generate_var() {
  ++var_count;
  return std::format("x{}", var_count);
}

std::tuple<Label, Label, Label> IRUtils::generate_labels(bool is_while) {
  ++label_count;
  return is_while ? std::make_tuple(std::format("while_start{}", label_count),
                                    std::format("while_body{}", label_count),
                                    std::format("while_end{}", label_count))

                  : std::make_tuple(std::format("then{}", label_count),
                                    std::format("if_end{}", label_count),
                                    std::format("else_end{}", label_count));
}

auto generate_ir(UPtrExpr &root) {
  std::unordered_map<IRVar, IRVar> symbol_table{};
  IRUtils utils{};

  IRGenerator ir_gen{utils};

  root->accept(ir_gen);

  return ir_gen.ins;
}
} // namespace compiler
