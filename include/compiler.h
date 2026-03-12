#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "datatypes.h"
#include "expression.h"
#include "ir.h"

namespace compiler {

std::vector<Token> tokenize(const std::string_view input);
UPtrExpr parse(const std::vector<Token> &tokens);
C_type typecheck(UPtrExpr &&root);
IRGenerator generate_ir(UPtrExpr &root);
std::vector<std::string> generate_assembly(IRGenerator &&ir_gen);

} // namespace compiler
