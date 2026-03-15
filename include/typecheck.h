#pragma once

#include <string>
#include <unordered_map>
#include <variant>

#include "symtab.h"

enum class C_type { C_int, C_bool, C_unit, C_any };

using FnType = std::vector<C_type>;
using SymbolType = std::variant<C_type, FnType>;
using TS_Scope = std::unordered_map<std::string, SymbolType>;

const TS_Scope kTypecheckGlobals{
    {"+",
     FnType{
         C_type::C_int,
         C_type::C_int,
         C_type::C_int,
     }},
    {"-",
     FnType{
         C_type::C_int,
         C_type::C_int,
         C_type::C_int,
     }},
    {"*",
     FnType{
         C_type::C_int,
         C_type::C_int,
         C_type::C_int,
     }},
    {"/",
     FnType{
         C_type::C_int,
         C_type::C_int,
         C_type::C_int,
     }},
    {"%",
     FnType{
         C_type::C_int,
         C_type::C_int,
         C_type::C_int,
     }},
    {"<",
     FnType{
         C_type::C_int,
         C_type::C_int,
         C_type::C_bool,
     }},
    {"<=",
     FnType{
         C_type::C_int,
         C_type::C_int,
         C_type::C_bool,
     }},
    {">",
     FnType{
         C_type::C_int,
         C_type::C_int,
         C_type::C_bool,
     }},
    {">=",
     FnType{
         C_type::C_int,
         C_type::C_int,
         C_type::C_bool,
     }},
    {"and",
     FnType{
         C_type::C_bool,
         C_type::C_bool,
         C_type::C_bool,
     }},
    {"or",
     FnType{
         C_type::C_bool,
         C_type::C_bool,
         C_type::C_bool,
     }},
    {"unary_-",
     FnType{
         C_type::C_int,
         C_type::C_int,
     }},
    {"unary_not",
     FnType{
         C_type::C_bool,
         C_type::C_bool,
     }},
    {"print_int",
     FnType{
         C_type::C_int,
         C_type::C_unit,
     }},
    {"print_bool",
     FnType{
         C_type::C_bool,
         C_type::C_unit,
     }},
    {"read_int",
     FnType{
         C_type::C_int,
     }},
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
  SymTab<std::string, SymbolType, TS_Scope> sym_tab{kTypecheckGlobals};

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
