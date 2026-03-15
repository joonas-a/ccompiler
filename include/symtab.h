#pragma once

#include <vector>

template <typename Key, typename Value, typename Scope> struct SymTab {
  std::vector<Scope> stack;

  explicit SymTab(Scope globals) : stack() {
    stack.reserve(1);
    stack.push_back(std::move(globals));
  }

  void add_scope() { stack.emplace_back(); }
  void remove_scope() { stack.pop_back(); }

  void add(const Key name, const Value symbol) {
    stack.back().emplace(name, symbol);
  }

  const Value *lookup(const Key &key) const {
    for (auto it = stack.rbegin(); it != stack.rend(); ++it)
      if (auto search = it->find(key); search != it->end())
        return &search->second;

    return nullptr;
  }

  bool local_key_exists(const Key &key) const {
    return stack.back().contains(key);
  }
};
