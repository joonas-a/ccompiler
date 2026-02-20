#include <algorithm>
#include <cassert>
#include <datatypes.h>
#include <functional>
#include <iostream>
#include <optional>
#include <variant>
#include <vector>

#include <errors.h>
#include <util.h>

using namespace std;

namespace compiler {

Expression parse(const vector<Token> &tokens) {
  ssize_t pos = 0;

  function<Expression()> parse_expr_left_assoc;

  auto peek = [&](ssize_t offset = 0) {
    if (tokens.empty())
      throw SyntaxError("Input cannot be empty");

    if (pos + offset < tokens.size())
      return tokens.at(pos + offset);

    if (pos + offset == tokens.size())
      return Token({tokens[-1].loc, Kind::END, ""});

    throw runtime_error("Peeking out of bounds");
  };

  auto consume =
      [&](optional<variant<string_view, vector<string_view>>> expected) {
        auto token = peek();

        println("{}", token.text);

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

  auto verify_syntax = [&]() {
    const Kind next = peek().type;
    if (next == Kind::INT_LITERAL || next == Kind::IDENTIFIER) {
      throw SyntaxError();
    }
  };

  auto parse_int_literal = [&]() {
    if ((pos > 0 && peek(-1).type == Kind::INT_LITERAL) ||
        peek(1).type == Kind::INT_LITERAL) {
      throw SyntaxError("Two int literals in a row");
    }

    auto token = consume(nullopt);
    return Literal{stoi(token.text)};
  };

  auto parse_identifier = [&]() {
    if ((pos > 0 && peek(-1).type == Kind::IDENTIFIER) ||
        peek(1).type == Kind::IDENTIFIER) {
      throw SyntaxError("Two identifiers in a row");
    }

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

    throw SyntaxError("Expected to find a suitable term, instead got: " +
                      token.text);
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

    verify_syntax();
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

    verify_syntax();
    return left;
  };

  return parse_expr_left_assoc();
}
} // namespace compiler
