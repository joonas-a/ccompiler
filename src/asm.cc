#include <format>
#include <iostream>
#include <limits>
#include <ostream>
#include <print>
#include <stdexcept>
#include <type_traits>

#include "asm.h"
#include "ir.h"

namespace compiler {

using std::format;

void Locals::init_stack(IRVarSet &ir_vars) {
  for (const auto var : ir_vars) {
    ++this->stack_used;
    this->ir_var_map.emplace(var, format("-{}(%rbp)", 8 * this->stack_used));
  }
}

std::string Locals::get_addr_for(IRVar var) {
  if (auto it = this->ir_var_map.find(var); it != this->ir_var_map.end()) {
    return it->second;
  }
  for (auto var : this->ir_var_map)

    std::cout << this->stack_used << "stack used" << std::endl;
  // std::cout << this-><< "stack used" << std::endl;

  throw std::runtime_error("Variable not allocated");
}

void AssemblyGenerator::start_boiler() {
  this->emit(".extern print_int");
  this->emit(".extern print_bool");
  this->emit(".extern read_int");
  this->emit("");
  this->emit(".section .text");
  this->emit("");
  this->emit(".global main");
  this->emit(".type main, @function");
  this->emit("");
  this->emit("main:");
  this->emit("");
  this->emit("pushq %rbp");
  this->emit("movq %rsp, %rbp");
  this->emit(format("subq ${}, %rsp", this->locals.stack_used * 8));
}

void AssemblyGenerator::end_boiler() {
  this->emit(".Lend");
  this->emit("movq $0, %rax");
  this->emit("movq %rbp, %rsp");
  this->emit("popq %rbp");
  this->emit("ret");
  this->emit("");

  // this->emit("scan_format:");
  // this->emit(".asciz \"%ld\"");
  // this->emit("print_format:");
  // this->emit(".asciz \"%ld\"\n");
}

void AssemblyGenerator::print() {
  for (auto &line : this->lines) {
    std::println("{}", line);
  }
}

void AssemblyGenerator::generate(std::vector<Instruction> &instructions) {
  for (const auto &instruction : instructions) {
    std::visit(
        [this](const auto &in) {
          using T = std::decay_t<decltype(in)>;

          if constexpr (std::is_same_v<T, Label>) {
            emit(format("# Label {}", in.text));
            emit(format(".L{}", in.text));

          } else if constexpr (std::is_same_v<T, LoadIntConst>) {
            // TODO: make sure bound checking works
            emit("# LoadIntConst");
            if (std::numeric_limits<int>::min() <= in.value &&
                in.value < std::numeric_limits<int>::max()) {
              emit(
                  format("movq {}, {}", in.value, locals.get_addr_for(in.dst)));
            } else {
              emit(format("movabsq {}, %rax", in.value));
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            }

          } else if constexpr (std::is_same_v<T, LoadBoolConst>) {
            emit("# LoadBoolConst");
            in.value ? emit(format("movq 1, {}", locals.get_addr_for(in.dst)))
                     : emit(format("movq 0, {}", locals.get_addr_for(in.dst)));

          } else if constexpr (std::is_same_v<T, Jump>) {
            emit("# Jump");
            emit(format("jmp .L{}", locals.get_addr_for(in.label.text)));

          } else if constexpr (std::is_same_v<T, Copy>) {
            emit("# Copy");
            emit(format("movq {}, %rax", locals.get_addr_for(in.src)));
            emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));

          } else if constexpr (std::is_same_v<T, CondJump>) {
            emit("# CondJump");
            emit(format("cmpq $0 {}", locals.get_addr_for(in.cond)));
            emit(format("jne .L{}", in.then_label.text));
            emit(format("jmp .L{}", in.else_label.text));
            // }

          } else if constexpr (std::is_same_v<T, Call>) {
            emit("# Call");

            if (in.fn == "+") {
              emit(format("movq {}, %rax", in.args[0]));
              emit(format("addq {}, %rax", in.args[1]));
              emit(format("movq %rax, {}", in.dst));
            } else if (in.fn == "-") {
              emit(format("movq {}, %rax", in.args[0]));
              emit(format("subq {}, %rax", in.args[1]));
              emit(format("movq %rax, {}", in.dst));
            } else if (in.fn == "*") {
              emit(format("movq {}, %rax", in.args[0]));
              emit(format("imulq {}, %rax", in.args[1]));
              emit(format("movq %rax, {}", in.dst));
            } else if (in.fn == "/") {
              emit(format("movq {}, %rax", in.args[0]));
              emit("cqto");
              emit(format("idivq {}", in.args[1]));
              emit(format("movq %rax, {}", in.dst));
            } else if (in.fn == "%") {
              emit(format("movq {}, %rax", in.args[0]));
              emit("cqto");
              emit(format("idivq {}", in.args[1]));
              emit(format("movq %rdx, %rax", in.dst));
              emit(format("movq %rax, {}", in.dst));
            } else if (in.fn == "==") {
              emit("xor %rax, %rax");
              emit(format("movq {}, %rdx", in.args[0]));
              emit(format("cmpq {}, %rdx", in.args[1]));
              emit("sete %al");
              emit(format("movq %rax, {}", in.dst));
            } else if (in.fn == "!=") {
              emit("xor %rax, %rax");
              emit(format("movq {}, %rdx", in.args[0]));
              emit(format("cmpq {}, %rdx", in.args[1]));
              emit("setne %al");
              emit(format("movq %rax, {}", in.dst));
            } else if (in.fn == "<") {
              emit("xor %rax, %rax");
              emit(format("movq {}, %rdx", in.args[0]));
              emit(format("cmpq {}, %rdx", in.args[1]));
              emit("setl %al");
              emit(format("movq %rax, {}", in.dst));
            } else if (in.fn == "<=") {
              emit("xor %rax, %rax");
              emit(format("movq {}, %rdx", in.args[0]));
              emit(format("cmpq {}, %rdx", in.args[1]));
              emit("setle %al");
              emit(format("movq %rax, {}", in.dst));
            } else if (in.fn == ">") {
              emit("xor %rax, %rax");
              emit(format("movq {}, %rdx", in.args[0]));
              emit(format("cmpq {}, %rdx", in.args[1]));
              emit("setg %al");
              emit(format("movq %rax, {}", in.dst));
            } else if (in.fn == ">=") {
              emit("xor %rax, %rax");
              emit(format("movq {}, %rdx", in.args[0]));
              emit(format("cmpq {}, %rdx", in.args[1]));
              emit("setge %al");
              emit(format("movq %rax, {}", in.dst));
            }
            emit("");
          }
        },
        instruction);
  }
}

auto generate_assembly(IRGenerator &&ir_gen) {
  Locals locals{};
  locals.init_stack(ir_gen.utils.ir_vars);

  AssemblyGenerator asm_gen{locals};

  asm_gen.start_boiler();

  asm_gen.generate(ir_gen.ins);

  asm_gen.end_boiler();

  asm_gen.print();

  return asm_gen.lines;
}

} // namespace compiler
