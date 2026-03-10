#include "ir.h"
#include "expression.h"

struct SymTab;

namespace compiler {

using IRVar = std::string;

IRGenerator::IRGenerator(SymTab &st) : symTab(st) {};

void Literal::accept(IRGenerator &tc) const {}
void Identifier::accept(IRGenerator &tc) const {}
void UnaryOp::accept(IRGenerator &tc) const {}
void BinaryOp::accept(IRGenerator &tc) const {}
void FunctionCall::accept(IRGenerator &tc) const {}
void IfThenStatement::accept(IRGenerator &tc) const {}
void IfThenElseStatement::accept(IRGenerator &tc) const {}
void Block::accept(IRGenerator &tc) const {}
void Variable::accept(IRGenerator &tc) const {}
void While::accept(IRGenerator &tc) const {}

} // namespace compiler
