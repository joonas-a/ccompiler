#include "expression.h"

struct SymTab;

namespace compiler {

using std::string;

using IRVar = string;

struct IRGenerator {
  SymTab &symTab;

  explicit IRGenerator(SymTab &st) : symTab(st) {};

  C_type visit(const Literal &e);
  C_type visit(const Identifier &e);
  C_type visit(const UnaryOp &e);
  C_type visit(const BinaryOp &e);
  C_type visit(const FunctionCall &e);
  C_type visit(const IfThenStatement &e);
  C_type visit(const IfThenElseStatement &e);
  C_type visit(const Block &e);
  C_type visit(const Variable &e);
  C_type visit(const While &e);
};

void Literal::accept(IRGenerator &tc) const { tc.visit(*this); }
void Identifier::accept(IRGenerator &tc) const { tc.visit(*this); }
void UnaryOp::accept(IRGenerator &tc) const { tc.visit(*this); }
void BinaryOp::accept(IRGenerator &tc) const { tc.visit(*this); }
void FunctionCall::accept(IRGenerator &tc) const { tc.visit(*this); }
void IfThenStatement::accept(IRGenerator &tc) const { tc.visit(*this); }
void IfThenElseStatement::accept(IRGenerator &tc) const { tc.visit(*this); }
void Block::accept(IRGenerator &tc) const { tc.visit(*this); }
void Variable::accept(IRGenerator &tc) const { tc.visit(*this); }
void While::accept(IRGenerator &tc) const { tc.visit(*this); }

} // namespace compiler
