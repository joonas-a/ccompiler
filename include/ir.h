#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace compiler {

using IRVar = std::string;
using Label = std::string;
using Labels = std::tuple<Label, Label, Label>;

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

struct IRUtils {
  size_t var_count{};
  size_t label_count{};

  std::unordered_set<IRVar> ir_vars{};
  IRVar generate_var();
  Labels generate_labels(bool is_while);
};

// IRGen instructions
struct LoadBoolConst {
  bool value;
  IRVar dest;
};

struct LoadIntConst {
  int value;
  IRVar dest;
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

struct IRGenerator {
  IRUtils &utils;
  std::unordered_map<std::string, IRVar> sym_tab{};
  std::vector<Instruction> ins{};

  explicit IRGenerator(IRUtils &utils);

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
