#pragma once

#include <string_view>
#include <vector>

#include "datatypes.h"
#include "expression.h"
#include "typecheck.h"

namespace compiler {

std::vector<Token> tokenize(const std::string_view input);
UPtrExpr parse(const std::vector<Token> &tokens);
C_type typecheck(UPtrExpr &root);

} // namespace compiler
