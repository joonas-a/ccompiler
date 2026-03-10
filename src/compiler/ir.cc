#include <type_traits>

#include "expression.h"
#include "ir.h"

struct SymTab;

namespace compiler {

IRGenerator::IRGenerator(SymTab &st, VarGenerator &vg)
    : symTab(st), varGen(vg) {};

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

        auto var = this->varGen.generate_var();
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

IRVar VarGenerator::generate_var() {
  ++counter;
  return std::format("x{}", counter);
}

IRVar generate_ir(UPtrExpr &root) {
  SymTab symbol_table{};
  VarGenerator var_gen{};

  IRGenerator ir_gen{symbol_table, var_gen};

  return root->accept(ir_gen);
}
} // namespace compiler
