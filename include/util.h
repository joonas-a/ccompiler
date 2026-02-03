#pragma once

#include <algorithm>
#include <string_view>

bool is_numeric(const std::string_view text);

template <typename Iterable>
bool find_in(std::string_view text, const Iterable &opts) {
  return std::ranges::contains(opts, text);
}
