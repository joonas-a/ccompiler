#pragma once

#include <stdexcept>
#include <string>

struct SyntaxError : std::runtime_error {

  SyntaxError() : std::runtime_error("Unexpected token") {};
  explicit SyntaxError(const std::string &msg) : std::runtime_error(msg) {}
  explicit SyntaxError(const char *msg) : std::runtime_error(msg) {}
};
