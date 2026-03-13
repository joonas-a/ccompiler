#include <format>
#include <limits>
#include <print>
#include <stdexcept>
#include <type_traits>

#include "asm.h"
#include "ir.h"

namespace compiler {

using std::format;

void Locals::init_stack(IRVarVec &ir_vars) {
  for (const auto var : ir_vars) {
    if (var == "unit")
      continue;
    ++this->stack_used;
    this->ir_var_map.emplace(var, format("-{}(%rbp)", 8 * this->stack_used));
  }
}

std::string Locals::get_addr_for(IRVar var) {
  if (auto it = this->ir_var_map.find(var); it != this->ir_var_map.end())
    return it->second;

  std::println("\n!!! Addr not found for {}", var);
  for (auto &x : ir_var_map)
    std::println("{} -> {}", x.first, x.second);

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
  this->emit_ir_vars();
  this->emit("");
  this->emit("pushq %rbp");
  this->emit("movq %rsp, %rbp");
  this->emit(format("subq ${}, %rsp", this->locals.stack_used * 8));
  this->emit("");
}

void AssemblyGenerator::end_boiler() {
  this->emit("# Return");
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

void AssemblyGenerator::print_asm() {
  for (auto &line : this->lines) {
    std::println("{}", line);
  }
}

void AssemblyGenerator::emit_ir_vars() {
  for (auto &var : this->locals.ir_var_map)
    this->emit(format("# {} in {}", var.first, var.second));
}

void AssemblyGenerator::generate(std::vector<Instruction> &instructions) {
  for (const auto &instruction : instructions) {
    std::visit(
        [this](const auto &in) {
          using T = std::decay_t<decltype(in)>;

          if constexpr (std::is_same_v<T, Label>) {
            emit(format(".L{}:", in.text));

          } else if constexpr (std::is_same_v<T, LoadIntConst>) {
            // TODO: make sure bound checking works
            emit("# LoadIntConst");
            if (std::numeric_limits<int>::min() <= in.value &&
                in.value < std::numeric_limits<int>::max()) {
              emit(format("movq ${}, {}", in.value,
                          locals.get_addr_for(in.dst)));
            } else {
              emit(format("movabsq ${}, %rax", in.value));
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            }

          } else if constexpr (std::is_same_v<T, LoadBoolConst>) {
            emit("# LoadBoolConst");
            in.value ? emit(format("movq $1, {}", locals.get_addr_for(in.dst)))
                     : emit(format("movq $0, {}", locals.get_addr_for(in.dst)));

          } else if constexpr (std::is_same_v<T, Jump>) {
            emit("# Jump");
            emit(format("jmp .L{}", in.label.text));

          } else if constexpr (std::is_same_v<T, Copy>) {
            emit("# Copy");
            emit(format("movq {}, %rax", locals.get_addr_for(in.src)));
            emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));

          } else if constexpr (std::is_same_v<T, CondJump>) {
            emit("# CondJump");
            emit(format("cmpq $0, {}", locals.get_addr_for(in.cond)));
            emit(format("jne .L{}", in.then_label.text));
            emit(format("jmp .L{}", in.else_label.text));

          } else if constexpr (std::is_same_v<T, Call>) {
            emit(format("# Call {}", in.fn));

            if (in.fn == "+") {
              emit(format("movq {}, %rax", locals.get_addr_for(in.args[0])));
              emit(format("addq {}, %rax", locals.get_addr_for(in.args[1])));
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            } else if (in.fn == "-") {
              emit(format("movq {}, %rax", locals.get_addr_for(in.args[0])));
              emit(format("subq {}, %rax", locals.get_addr_for(in.args[1])));
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            } else if (in.fn == "*") {
              emit(format("movq {}, %rax", locals.get_addr_for(in.args[0])));
              emit(format("imulq {}, %rax", locals.get_addr_for(in.args[1])));
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            } else if (in.fn == "/") {
              emit(format("movq {}, %rax", locals.get_addr_for(in.args[0])));
              emit("cqto");
              emit(format("idivq {}", locals.get_addr_for(in.args[1])));
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            } else if (in.fn == "%") {
              emit(format("movq {}, %rax", locals.get_addr_for(in.args[0])));
              emit("cqto");
              emit(format("idivq {}", locals.get_addr_for(in.args[1])));
              emit(format("movq %rdx, %rax", locals.get_addr_for(in.dst)));
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            } else if (in.fn == "==") {
              emit("xor %rax, %rax");
              emit(format("movq {}, %rdx", locals.get_addr_for(in.args[0])));
              emit(format("cmpq {}, %rdx", locals.get_addr_for(in.args[1])));
              emit("sete %al");
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            } else if (in.fn == "!=") {
              emit("xor %rax, %rax");
              emit(format("movq {}, %rdx", locals.get_addr_for(in.args[0])));
              emit(format("cmpq {}, %rdx", locals.get_addr_for(in.args[1])));
              emit("setne %al");
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            } else if (in.fn == "<") {
              emit("xor %rax, %rax");
              emit(format("movq {}, %rdx", locals.get_addr_for(in.args[0])));
              emit(format("cmpq {}, %rdx", locals.get_addr_for(in.args[1])));
              emit("setl %al");
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            } else if (in.fn == "<=") {
              emit("xor %rax, %rax");
              emit(format("movq {}, %rdx", locals.get_addr_for(in.args[0])));
              emit(format("cmpq {}, %rdx", locals.get_addr_for(in.args[1])));
              emit("setle %al");
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            } else if (in.fn == ">") {
              emit("xor %rax, %rax");
              emit(format("movq {}, %rdx", locals.get_addr_for(in.args[0])));
              emit(format("cmpq {}, %rdx", locals.get_addr_for(in.args[1])));
              emit("setg %al");
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            } else if (in.fn == ">=") {
              emit("xor %rax, %rax");
              emit(format("movq {}, %rdx", locals.get_addr_for(in.args[0])));
              emit(format("cmpq {}, %rdx", locals.get_addr_for(in.args[1])));
              emit("setge %al");
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            } else if (in.fn == "unary_-") {
              emit(format("movq {}, %rax", locals.get_addr_for(in.args[0])));
              emit(format("negq %rax"));
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            } else if (in.fn == "unary_not") {
              emit(format("movq {}, %rax", locals.get_addr_for(in.args[0])));
              emit(format("xorq $1, %rax"));
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            } else if (in.fn == "print_int") {
              emit(format("movq {}, %rdi", locals.get_addr_for(in.args[0])));
              emit(format("callq print_int"));
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            } else if (in.fn == "print_bool") {
              emit(format("movq {}, %rdi", locals.get_addr_for(in.args[0])));
              emit(format("callq print_bool"));
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            } else if (in.fn == "read_int") {
              emit(format("callq read_int"));
              emit(format("movq %rax, {}", locals.get_addr_for(in.dst)));
            }
          }
          emit("");
        },
        instruction);
  };
}

auto generate_assembly(IRGenerator &&ir_gen) {
  Locals locals{};
  locals.init_stack(ir_gen.utils.ir_vars);

  AssemblyGenerator asm_gen{locals};

  std::println("# IRVars = {}", ir_gen.utils.ir_vars.size());

  asm_gen.start_boiler();

  asm_gen.generate(ir_gen.ins);

  asm_gen.end_boiler();

  asm_gen.print_asm();

  return asm_gen.lines;
}

} // namespace compiler
