#include <cassert>
#include <datatypes.h>
#include <functional>
#include <optional>
#include <variant>
#include <vector>

#include <errors.h>
#include <util.h>

using namespace std;

namespace compiler {

Expression parse(const vector<Token> &tokens) {
  size_t pos = 0;

  function<Expression()> parse_expr_left_assoc;

  auto peek = [&]() {
    if (tokens.empty())
      throw SyntaxError("Input cannot be empty");
    if (pos != tokens.size())
      return tokens.at(pos);
    return Token({tokens[-1].loc, Kind::END, ""});
  };

  auto consume =
      [&](optional<variant<string_view, vector<string_view>>> expected) {
        auto token = peek();

        if (expected.has_value()) {
          if (expected->index() == 0) {
            if (get<0>(*expected) != token.text) {
              throw SyntaxError("Expected something other than: " + token.text);
            }
          } else {
            if (!find_in(token.text, get<1>(*expected)))
              throw SyntaxError("Unexpected token: " + token.text);
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

  auto parse_identifier = [&]() {
    if (peek().type != Kind::IDENTIFIER)
      throw SyntaxError();
    auto token = consume(nullopt);
    return Identifier{token.text};
  };

  auto parse_parenthesized = [&]() {
    consume("(");
    const auto expr = parse_expr_left_assoc();
    consume(")");
    return expr;
  };

  auto parse_factor = [&]() -> Expression {
    auto token = peek();
    if (token.text == "(")
      return parse_parenthesized();
    if (token.type == Kind::INT_LITERAL)
      return parse_int_literal();
    if (token.type == Kind::IDENTIFIER)
      return parse_identifier();
    throw SyntaxError("Expected to find a suitable term, instead got: " + token.text);
  };

  auto parse_term = [&]() -> Expression {
    const vector<string_view> ops{"*", "/"}; // FIXME
    auto left = parse_factor();

    while (find_in(peek().text, ops)) {
      const auto op_token = consume(nullopt);
      const auto op = op_token.text;

      const auto right = parse_factor();

      left = BinaryOp{left, op, right};
    };
    return left;
  };

  parse_expr_left_assoc = [&]() {
    const vector<string_view> ops{"+", "-"}; // FIXME
    auto left = parse_term();

    while (find_in(peek().text, ops)) {
      const auto op_token = consume(nullopt);
      const auto op = op_token.text;

      const auto right = parse_term();

      left = BinaryOp{left, op, right};
    };

    return left;
  };

  return parse_expr_left_assoc();
}
} // namespace compiler
