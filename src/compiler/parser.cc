#include <cassert>
#include <datatypes.h>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include <errors.h>
#include <util.h>

using namespace std;

namespace compiler {

static const vector<vector<string_view>> left_assoc_binary_ops = {
    vector<string_view>{"or"},       vector<string_view>{"and"},
    vector<string_view>{"==", "!="}, vector<string_view>{"<", "<=", ">", ">="},
    vector<string_view>{"+", "-"},   vector<string_view>{"*", "/", "%"},
};

unique_ptr<Expression> parse(const vector<Token> &tokens) {
  ssize_t pos = 0;

  function<unique_ptr<Expression>()> parse_expr_left_assoc;

  auto peek = [&](ssize_t offset = 0) -> const Token & {
    if (tokens.empty())
      throw SyntaxError("Input cannot be empty");

    if (pos + offset < static_cast<ssize_t>(tokens.size())) {
      // println("Peeking: {}", tokens.at(pos + offset).text);
      return tokens.at(pos + offset);
    }

    if (pos + offset == static_cast<ssize_t>(tokens.size())) {
      static Token end{tokens.back().loc, Kind::END, ""};
      return end;
    }

    throw runtime_error("Out of bounds!");
  };

  auto consume =
      [&](optional<variant<string_view, vector<string_view>>> expected) {
        const auto &token = peek();

        // println("Consuming {}", token.text);

        if (expected.has_value()) {
          if (expected->index() == 0) {
            if (get<0>(*expected) != token.text) {
              throw SyntaxError("Unexpected token: " + token.as_string());
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
    return make_unique<Literal>(stoi(token.text));
  };

  auto parse_function = [&](unique_ptr<Identifier> name) {
    consume("(");
    vector<unique_ptr<Expression>> args{};
    args.emplace_back(parse_expr_left_assoc());

    while (peek().text == ",") {
      consume(",");
      args.emplace_back(parse_expr_left_assoc());
    }

    consume(")");
    return make_unique<FunctionCall>(std::move(name), std::move(args));
  };

  auto parse_identifier = [&]() -> unique_ptr<Expression> {
    if ((pos > 0 && peek(-1).type == Kind::IDENTIFIER) ||
        peek(1).type == Kind::IDENTIFIER) {
      throw SyntaxError("Two identifiers in a row");
    }

    auto token = consume(nullopt);

    if (peek().text == "(") {
      return parse_function(make_unique<Identifier>(token.text));
    }

    return make_unique<Identifier>(token.text);
  };

  auto parse_parenthesized = [&]() {
    consume("(");
    auto expr = parse_expr_left_assoc();
    consume(")");
    return expr;
  };

  auto parse_conditional = [&]() -> unique_ptr<Expression> {
    consume("if");
    auto condition = parse_expr_left_assoc();
    consume("then");
    auto then_branch = parse_expr_left_assoc();

    if (peek().text == "else") {
      consume(nullopt);
      auto else_branch = parse_expr_left_assoc();

      return make_unique<IfThenElseStatement>(
          std::move(condition), std::move(then_branch), std::move(else_branch));
    }

    return make_unique<IfThenStatement>(std::move(condition),
                                        std::move(then_branch));
  };

  auto parse_factor = [&]() -> unique_ptr<Expression> {
    auto token = peek();
    if (token.text == "(")
      return parse_parenthesized();

    if (token.type == Kind::INT_LITERAL)
      return parse_int_literal();

    if (token.type == Kind::IDENTIFIER)
      return parse_identifier();

    if (token.type == Kind::CONDITIONAL)
      return parse_conditional();

    throw SyntaxError("Expected to find a suitable term, instead got: " +
                      token.text);
  };

  parse_expr_left_assoc = [&]() {
    auto left = parse_factor();

    for (size_t prec = 5; prec >= 3; --prec) {
      while (find_in(peek().text, left_assoc_binary_ops[prec])) {
        const auto op_token = consume(nullopt);
        auto op = op_token.text;

        auto right = parse_factor();

        left = make_unique<BinaryOp>(std::move(left), op, std::move(right));
      }
    }

    verify_syntax();
    return left;
  };

  return parse_expr_left_assoc();
}
} // namespace compiler
