#pragma once

#include <stdexcept>
#include <string>

struct SyntaxError : std::runtime_error {

  SyntaxError() noexcept : std::runtime_error("Unexpected token") {};
  explicit SyntaxError(const std::string &msg) noexcept
      : std::runtime_error(msg) {}
};
