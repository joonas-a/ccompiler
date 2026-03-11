#include <format>
#include <type_traits>

#include "asm.h"
#include "ir.h"

namespace compiler {

using std::format;

void Locals::init_stack(IRVarSet ir_vars) {
  for (auto &var : ir_vars) {
    ++this->stack_used;
    this->ir_var_map.emplace(var, format("-{}(%rbp)", 8 * this->stack_used));
  }
}

void AssemblyGenerator::insert_boiler() {
  this->emit(".extern print_int");
  this->emit(".extern print_bool");
  this->emit(".extern read_int");
  this->emit(".global main");
  this->emit(".type main, @function");

  this->emit("");

  this->emit(".section .text");

  this->emit("");

  this->emit("main:");
  this->emit("pushq %rbp");
  this->emit("movq %rsp, %rbp");
  this->emit(format("subq %{}, %rsp", this->locals.stack_used));
}

void AssemblyGenerator::generate(std::vector<Instruction> &instructions) {
  for (const auto &instruction : instructions) {
    std::visit(
        [this](const auto &in) {
          using T = std::decay_t<decltype(in)>;
          if constexpr (std::is_same_v<T, Label>) {
            emit(format("# {}\n", in.text));
            emit(format(""));
          }
        },
        instruction);
  }
}

auto generate_assembly(IRGenerator ir_gen) {
  Locals locals{};
  locals.init_stack(ir_gen.utils.ir_vars);
  AssemblyGenerator asm_gen{locals};

  asm_gen.insert_boiler();
  asm_gen.generate(ir_gen.ins);
}

} // namespace compiler
