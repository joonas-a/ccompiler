#pragma once

#include <datatypes.h>
#include <string_view>
#include <vector>

namespace compiler {

std::vector<Token> tokenize(const std::string_view input);
std::unique_ptr<Expression> parse(const std::vector<Token> &tokens);

} // namespace compiler
