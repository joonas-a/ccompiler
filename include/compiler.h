#pragma once

#include <string_view>
#include <vector>

#include "datatypes.h"
#include "expression.h"

namespace compiler {

std::vector<Token> tokenize(const std::string_view input);
std::unique_ptr<Expression> parse(const std::vector<Token> &tokens);

} // namespace compiler
