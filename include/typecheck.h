#pragma once

#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "symtab.h"

struct C_int {};
struct C_bool {};
struct C_unit {};
struct C_fn;

using C_type = std::variant<C_int, C_bool, C_unit, std::shared_ptr<C_fn>>;

struct C_fn {
  std::vector<C_type> args;
  C_type return_type;
};

// using SymbolType = std::variant<C_Primitive, FnType>;
using TS_Scope = std::unordered_map<std::string, C_type>;

inline C_type make_fn(std::initializer_list<C_type> args, C_type ret) {
  return std::make_unique<C_fn>(
      C_fn{std::move(std::vector<C_type>(args)), std::move(ret)});
}

inline auto get_fn(const C_type &ct) {
  return std::get_if<std::shared_ptr<C_fn>>(&ct);
}

const TS_Scope kTypecheckGlobals{
    {"+", make_fn({C_int{}, C_int{}}, C_int{})},
    {"-", make_fn({C_int{}, C_int{}}, C_int{})},
    {"*", make_fn({C_int{}, C_int{}}, C_int{})},
    {"/", make_fn({C_int{}, C_int{}}, C_int{})},
    {"%", make_fn({C_int{}, C_int{}}, C_int{})},
    {"<", make_fn({C_int{}, C_int{}}, C_bool{})},
    {"<=", make_fn({C_int{}, C_int{}}, C_bool{})},
    {">", make_fn({C_int{}, C_int{}}, C_bool{})},
    {">=", make_fn({C_int{}, C_int{}}, C_bool{})},
    {"and", make_fn({C_bool{}, C_bool{}}, C_bool{})},
    {"or", make_fn({C_bool{}, C_bool{}}, C_bool{})},
    {"unary_-", make_fn({C_int{}}, C_int{})},
    {"unary_not", make_fn({C_bool{}}, C_bool{})},
    {"print_int", make_fn({C_int{}}, C_unit{})},
    {"print_bool", make_fn({C_bool{}}, C_unit{})},
    {"read_int", make_fn({}, C_int{})},
};

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
  SymTab<std::string, C_type, TS_Scope> sym_tab{kTypecheckGlobals};

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
