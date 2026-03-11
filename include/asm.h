#pragma once

#include <cstddef>
#include <unordered_map>
#include <unordered_set>

#include "ir.h"

namespace compiler {

using IRVarSet = std::unordered_set<IRVar>;

struct Locals {
  size_t stack_used{};
  std::unordered_map<IRVar, std::string> ir_var_map{};

  void init_stack(IRVarSet ir_vars);
};

} // namespace compiler
