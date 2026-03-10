#pragma once

#include <string>
#include <vector>

#include "symtab.h"

namespace compiler {

using IRVar = std::string;
using Label = std::string;

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

struct VarGenerator {
  unsigned long long counter{};

  IRVar generate_var();
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

using Instruction =
    std::variant<LoadBoolConst, LoadIntConst, Copy, Call, Jump, CondJump>;


struct IRGenerator {
  SymTab &symTab;
  VarGenerator &varGen;
  std::vector<Instruction> ins{};

  explicit IRGenerator(SymTab &st, VarGenerator& vg);

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
