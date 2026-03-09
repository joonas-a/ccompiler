#pragma once

#include <unordered_map>
#include <variant>
#include <vector>

// === Typechecker types ===
enum class C_type { C_int, C_bool, C_unit, C_any };

using FnType = std::vector<C_type>;
using SymEntry = std::variant<C_type, FnType>;
using Scope = std::unordered_map<std::string, SymEntry>;

Scope initialize_globals();
