#pragma once

#include "symtab.h"
#include <map>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace compiler {

using IRVar = std::string;

struct Literal;
struct Identifier;
struct UnaryOp;
struct BinaryOp;
struct FunctionCall;
struct IfThenStatement;
struct IfThenElseStatement;
struct Block;
struct Variable;
struct While;

// IRGen instructions
struct Label {
  std::string text{};
  Label(std::string t) : text(std::move(t)) {};
};

struct LoadBoolConst {
  bool value;
  IRVar dst;
};

struct LoadIntConst {
  unsigned long value;
  IRVar dst;
};

struct Copy {
  IRVar src;
  IRVar dst;
};

struct Call {
  IRVar fn;
  std::vector<IRVar> args;
  IRVar dst;
};

struct Jump {
  Label label;
};

struct CondJump {
  IRVar cond;
  Label then_label;
  Label else_label;
};

using Instruction = std::variant<LoadBoolConst, LoadIntConst, Copy, Call, Jump,
                                 CondJump, Label>;
using Labels = std::tuple<Label, Label, Label>;

struct IRUtils {
  size_t var_count{};
  size_t label_count{};
  bool unit_dispatched{false};

  std::vector<IRVar> ir_vars{};
  IRVar generate_var();
  IRVar generate_unit();
  Labels generate_labels(std::string_view keyword);
};

using IR_Scope = std::map<std::string, IRVar>;

static const IR_Scope kIRGlobals{
    {"print_int", "print_int"},
    {"print_bool", "print_bool"},
    {"read_int", "read_int"},
};

struct IRGenerator {
  IRUtils utils{};
  SymTab<std::string, IRVar, IR_Scope> sym_tab{kIRGlobals};
  std::vector<Instruction> ins{};

  IRVar visit(const Literal &e);
  IRVar visit(const Identifier &e);
  IRVar visit(const UnaryOp &e);
  IRVar visit(const BinaryOp &e);
  IRVar visit(const FunctionCall &e);
  IRVar visit(const IfThenStatement &e);
  IRVar visit(const IfThenElseStatement &e);
  IRVar visit(const Block &e);
  IRVar visit(const Variable &e);
  IRVar visit(const While &e);
};

} // namespace compiler
