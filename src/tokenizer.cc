#include <algorithm>
#include <cctype>
#include <cstddef>
#include <ranges>
#include <regex>
#include <string_view>
#include <vector>

#include "datatypes.h"

namespace compiler {

constexpr std::array<std::string_view, 15> kOperators = {
    "+",  "-", "*", "/", "=",   "==", "!=", "<=",
    ">=", "<", ">", "%", "and", "or", "not"};
constexpr std::array<std::string_view, 7> kPunctuators = {"(", ")", "{", "}",
                                                          ",", ";", ":"};
constexpr std::array<std::string_view, 2> kComments = {"//", "#"};
constexpr std::array<std::string_view, 3> kConditionals = {"if", "then",
                                                           "else"};

constexpr static inline bool is_numeric(const std::string_view text) {
  return std::ranges::all_of(text, [](char x) { return std::isdigit(x); });
}

static constexpr Kind get_type(std::string_view text) {
  using std::ranges::contains;

  if (is_numeric(text))
    return Kind::INT_LITERAL;
  if (contains(kOperators, text))
    return Kind::OPERATOR;
  if (contains(kConditionals, text))
    return Kind::CONDITIONAL;
  if (contains(kPunctuators, text))
    return Kind::PUNCTUATOR;
  if (contains(kComments, text))
    return Kind::COMMENT;

  return Kind::IDENTIFIER;
}

std::vector<Token> tokenize(const std::string_view input) {
  std::vector<Token> all_tokens{};
  all_tokens.emplace_back(Token{{0, 0}, Kind::PUNCTUATOR, "{"});

  constexpr auto delim{std::string_view{"\n"}};
  const auto tokenizer_regex =
      std::regex("[a-zA-Z_]+[a-zA-Z_0-9]*|[0-9]+|//|==|!=|<=|>"
                 "=|[//+-//*/%=<>//(//)//{//},;:#]{1}");

  size_t line_num = 1;
  for (const auto line : std::views::split(input, delim)) {
    auto it = std::regex_iterator<std::string_view::iterator>{
        line.begin(), line.end(), tokenizer_regex};

    for (decltype(it) last; it != last; ++it) {
      auto match = it->str();
      auto pos = it->position();
      auto type = get_type(match);

      if (type == Kind::COMMENT)
        break;

      all_tokens.emplace_back(Token({{line_num, 1}, type, match}));
    }
    ++line_num;
  }

  all_tokens.emplace_back(Token{{line_num, 0}, Kind::PUNCTUATOR, "}"});
  all_tokens.emplace_back(Token{{line_num, 1}, Kind::END, "$SENTINEL"});
  return all_tokens;
}

} // namespace compiler
