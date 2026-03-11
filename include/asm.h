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

struct AssemblyGenerator {
  Locals &locals;
  std::vector<std::string> lines{};

  explicit AssemblyGenerator(Locals &locals) : locals(locals) {};

  void emit(std::string line) { lines.push_back(line); }

  void insert_boiler();
  void generate(std::vector<Instruction> &instructions);
};

} // namespace compiler
