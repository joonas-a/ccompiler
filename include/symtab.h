#pragma once

#include <string>
#include <vector>

#include "datatypes.h"

struct SymTab {
  std::vector<Scope> stack;

  SymTab();

  void add_scope();
  void remove_scope();
  void add(std::string name, C_type symbol);

  const SymEntry *lookup(const std::string &key) const;

  bool local_key_exists(const std::string &key) const;
};
