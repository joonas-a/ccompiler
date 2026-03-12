#include <print>

#include "datatypes.h"
#include "symtab.h"

const Scope kGlobals{{"+",
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
                     {"=",
                      FnType{
                          C_type::C_any,
                          C_type::C_any,
                      }},
                     {"unary_-",
                      FnType{
                          C_type::C_int,
                          C_type::C_int,
                      }},
                     {"unary_not", FnType{
                                       C_type::C_bool,
                                       C_type::C_bool,
                                   }}};

SymTab::SymTab() : stack{kGlobals} {}

void SymTab::add_scope() { stack.emplace_back(); }
void SymTab::remove_scope() { stack.pop_back(); }

void SymTab::add(std::string name, C_type symbol) {
  // std::println("Adding {} with enum val {}", name, static_cast<int>(symbol));
  stack.back().emplace(std::move(name), symbol);
}

const SymEntry *SymTab::lookup(const std::string &key) const {
  for (auto it = stack.rbegin(); it != stack.rend(); ++it)
    if (auto search = it->find(key); search != it->end())
      return &search->second;

  return nullptr;
}

bool SymTab::local_key_exists(const std::string &key) const {
  return stack.back().contains(key);
}
