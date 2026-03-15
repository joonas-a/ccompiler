#pragma once

#include <cstddef>
#include <map>
#include <vector>

#include "ir.h"

namespace compiler {

using IRVarVec = std::vector<IRVar>;

struct Locals {
  size_t stack_used{};
  std::map<IRVar, std::string> ir_var_map{};

  void init_stack(IRVarVec &ir_vars);
  std::string get_addr_for(IRVar var);
};

struct AssemblyGenerator {
  Locals &locals;
  std::vector<std::string> lines{};

  explicit AssemblyGenerator(Locals &locals) : locals(locals) {};

  void emit(std::string line) { lines.push_back(line); }

  void start_boiler();
  void end_boiler();
  void generate(std::vector<Instruction> &instructions);
  void emit_ir_vars();
  void print_asm();
  void call_function(Call call);
};

} // namespace compiler
