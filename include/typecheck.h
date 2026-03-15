#pragma once

#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "symtab.h"

struct C_int {
  friend bool operator==(const C_int &a, const C_int &b) = default;
};
struct C_bool {
  friend bool operator==(const C_bool &a, const C_bool &b) = default;
};
struct C_unit {
  friend bool operator==(const C_unit &a, const C_unit &b) = default;
};

struct C_fn;

using C_type = std::variant<C_int, C_bool, C_unit, std::shared_ptr<C_fn>>;

bool equals(const C_type &a, const C_type &b);

struct C_fn {
  std::vector<C_type> args;
  C_type return_type;

  friend bool operator==(const C_fn &a, const C_fn &b) {
    if (a.args.size() != b.args.size())
      return false;
    for (size_t i = 0; i < a.args.size(); ++i) {
      if (!equals(a.args[i], b.args[i]))
        return false;
    }
    return (equals(a.return_type, b.return_type));
  }
};

inline bool operator==(const C_type a, const C_type b) { return equals(a, b); }

inline bool equals(const C_type &a, const C_type &b) {
  if (a.index() != b.index())
    return false;

  return std::visit(
      [&](const auto &av) -> bool {
        using T = std::decay_t<decltype(av)>;
        if constexpr (std::is_same_v<T, std::shared_ptr<C_fn>>) {
          return *av == *std::get<std::shared_ptr<C_fn>>(b);
        } else {
          return av == std::get<T>(b);
        }
      },
      a);
}

using TS_Scope = std::unordered_map<std::string, C_type>;

inline C_type make_fn(std::vector<C_type> args, C_type ret) {
  return std::make_unique<C_fn>(C_fn{std::move(args), std::move(ret)});
}
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
