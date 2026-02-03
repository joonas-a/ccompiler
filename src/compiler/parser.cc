#include <cassert>
#include <datatypes.h>
#include <optional>
#include <variant>
#include <vector>

#include <errors.h>
#include <util.h>

using namespace std;

namespace compiler {

Expression parse(vector<Token> &tokens) {
  size_t pos = 0;

  auto peek = [&]() {
    if (pos != tokens.size())
      return tokens.at(pos);
    return Token({tokens[-1].loc, Kind::END, ""});
  };

  auto consume =
      [&](optional<variant<string_view, vector<string_view>>> expected) {
        auto token = peek();

        if (expected.has_value()) {
          if (expected->index() == 0 && get<0>(*expected) != token.text) {
            throw SyntaxError();
          }

          if (!find_in(token.text, get<1>(*expected))) {
            throw SyntaxError();
          }
        }

        ++pos;
        return token;
      };

  auto parse_int_literal = [&]() {
    if (peek().type != Kind::INT_LITERAL)
      throw SyntaxError();
    auto token = consume(nullopt);
    return Literal{stoi(token.text)};
  };

  auto parse_expression = [&]() {
    const vector<string_view> ops{"+", "-"};
    const auto left = parse_int_literal();
    const auto op_token = consume(ops);
    const auto right = parse_int_literal();
    return BinaryOp{left, op_token.text, right};
  };

  return parse_expression();
}
} // namespace compiler
