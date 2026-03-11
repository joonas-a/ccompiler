#pragma once

#include "datatypes.h"
#include "symtab.h"

namespace compiler {

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

struct TypeChecker {
  SymTab &sym_tab;

  explicit TypeChecker(SymTab &st);

  C_type visit(const Literal &e);
  C_type visit(const Identifier &e);
  C_type visit(const UnaryOp &e);
  C_type visit(const BinaryOp &e);
  C_type visit(const FunctionCall &e);
  C_type visit(const IfThenStatement &e);
  C_type visit(const IfThenElseStatement &e);
  C_type visit(const Block &e);
  C_type visit(const Variable &e);
  C_type visit(const While &e);
};

} // namespace compiler
