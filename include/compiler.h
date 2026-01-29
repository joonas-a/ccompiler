#pragma once

#include <datatypes.h>
#include <string_view>
#include <vector>

namespace compiler {

std::vector<Token> tokenize(const std::string_view input);

}
