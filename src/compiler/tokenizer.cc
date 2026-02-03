#include <algorithm>
#include <array>
#include <cstdio>
#include <print>
#include <ranges>
#include <regex>
#include <string_view>
#include <vector>

#include <datatypes.h>

using namespace std;

static constexpr array<string_view, 12> OPERATORS = {
    "+", "-", "*", "/", "=", "==", "!=", "<=", ">=", "<", ">", "%"};
static constexpr array<string_view, 6> PUNCTUATORS = {"(", ")", "{",
                                                      "}", ",", ";"};
static constexpr array<string_view, 2> COMMENTS = {"//", "#"};
static constexpr array<string_view, 3> CONDITIONALS = {"if", "then", "else"};

bool is_numeric(const string_view text) {
  return ranges::all_of(text, [](char x) { return !!isdigit(x); });
}

template <typename Iterable>
bool find_in(string_view text, const Iterable &opts) {
  return ranges::contains(opts, text);
}

static Kind get_type(const string_view text) {
  if (is_numeric(text))
    return Kind::INT_LITERAL;
  if (find_in(text, OPERATORS))
    return Kind::OPERATOR;
  if (find_in(text, PUNCTUATORS))
    return Kind::PUNCTUATOR;
  if (find_in(text, COMMENTS))
    return Kind::COMMENT;
  if (find_in(text, CONDITIONALS))
    return Kind::CONDITIONAL;

  return Kind::IDENTIFIER;
}

namespace compiler {

vector<Token> tokenize(const string_view input) {
  vector<Token> all_tokens;

  constexpr auto delim{"\n"sv};
  const auto tokenizer_regex =
      regex("[a-zA-Z_]+[a-zA-Z_0-9]*|[0-9]+|//|==|!=|<=|>"
            "=|[//+-//*/%=<>//(//)//{//},;#]{1}");

  size_t line_num = 1;
  for (const auto line : views::split(input, delim)) {
    auto it = regex_iterator<string_view::iterator>{line.begin(), line.end(),
                                                    tokenizer_regex};

    for (decltype(it) last; it != last; ++it) {
      auto match = it->str();
      auto pos = it->position();
      auto type = get_type(match);

      if (type == Kind::COMMENT)
        break;

      all_tokens.push_back(Token({{line_num, 1}, type, match}));
    }
    ++line_num;
  }

  return all_tokens;
}

} // namespace compiler
