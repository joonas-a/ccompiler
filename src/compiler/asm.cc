#include "asm.h"
#include "ir.h"
#include <format>

namespace compiler {

void Locals::init_stack(IRVarSet ir_vars) {
  for (auto &var : ir_vars) {
    ++this->stack_used;
    this->ir_var_map.emplace(var,
                              std::format("-{}(%rbp)", 8 * this->stack_used));
  }
}

auto generate_assembly(IRGenerator ir_gen) { return; }

} // namespace compiler
