#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
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

  std::vector<IRVar> ir_vars{};
  IRVar generate_var();
  Labels generate_labels(std::string_view keyword);
};

struct IRGenerator {
  IRUtils utils{};
  std::unordered_map<std::string, IRVar> sym_tab{};
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
