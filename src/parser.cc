#include <cassert>
#include <datatypes.h>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "errors.h"
#include "expression.h"

using namespace std;

namespace compiler {

static const vector<vector<string_view>> la_binary_ops = {
    vector<string_view>{"or"},       vector<string_view>{"and"},
    vector<string_view>{"==", "!="}, vector<string_view>{"<", "<=", ">", ">="},
    vector<string_view>{"+", "-"},   vector<string_view>{"*", "/", "%"},
};

UPtrExpr parse(const vector<Token> &tokens) {
  ssize_t pos = 0;

  function<UPtrExpr()> parse_expr;
  function<UPtrExpr()> parse_factor;

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

  auto consume = [&](optional<variant<string, vector<string_view>>> expected) {
    const auto &token = peek();

    if (expected.has_value()) {
      if (expected->index() == 0) {
        if (get<0>(*expected) != token.text) {
          throw SyntaxError("Consume: expected " + get<0>(*expected) +
                            " but found: " + token.as_string());
        }
      } else {
        if (!std::ranges::contains(get<1>(*expected), token.text))
          throw SyntaxError("Consume: Unexpected token: " + token.text);
      }
    }

    ++pos;
    return token;
  };

  auto verify_syntax = [&]() {
    const Kind next = peek().type;
    if (peek(-1).text == "}" || peek().text == "do")
      return;
    if (next == Kind::INT_LITERAL || next == Kind::IDENTIFIER) {
      throw SyntaxError("Unexpected tokens; " + peek().text + peek(1).text);
    }
  };

  auto parse_int_literal = [&]() {
    if ((pos > 0 && peek(-1).type == Kind::INT_LITERAL) ||
        peek(1).type == Kind::INT_LITERAL) {
      throw SyntaxError("Two int literals in a row");
    }

    auto token = consume(nullopt);
    return make_unique<Literal>(stol(token.text));
  };

  auto parse_function = [&](unique_ptr<Identifier> name) {
    consume("(");
    vector<UPtrExpr> args{};

    if (peek().text != ")") {
      args.emplace_back(parse_expr());

      while (peek().text == ",") {
        consume(",");
        args.emplace_back(parse_expr());
      }
    }

    consume(")");
    return make_unique<FunctionCall>(std::move(name), std::move(args));
  };

  auto parse_identifier = [&]() -> UPtrExpr {
    if ((pos > 0 && peek(-1).type == Kind::IDENTIFIER) ||
        peek(1).type == Kind::IDENTIFIER) {
      throw SyntaxError("Two identifiers in a row");
    }

    auto token = consume(nullopt);

    if (peek().text == "(") {
      return parse_function(make_unique<Identifier>(token.text));
    }

    if (token.text == "true" || token.text == "false") {
      return make_unique<Literal>(token.text == "true" ? true : false);
    }
    return make_unique<Identifier>(token.text);
  };

  auto parse_parenthesized = [&]() {
    consume("(");
    auto expr = parse_expr();
    consume(")");
    return expr;
  };

  auto parse_conditional = [&]() -> UPtrExpr {
    consume("if");
    auto condition = parse_expr();
    consume("then");
    auto then_branch = parse_expr();

    if (peek().text == "else") {
      consume(nullopt);
      auto else_branch = parse_expr();

      return make_unique<IfThenElseStatement>(
          std::move(condition), std::move(then_branch), std::move(else_branch));
    }

    return make_unique<IfThenStatement>(std::move(condition),
                                        std::move(then_branch));
  };

  auto parse_unary = [&]() {
    consume(vector<string_view>{"-", "not"});
    auto op = peek(-1).text;
    return make_unique<UnaryOp>(op, parse_factor());
  };

  auto parse_variable = [&]() {
    consume("var");
    auto token = consume(nullopt);
    if (token.type != Kind::IDENTIFIER) {
      throw SyntaxError("Expected identifier for variable name, found: " +
                        token.as_string());
    }
    consume("=");
    return make_unique<Variable>(token.text, parse_expr());
  };

  auto parse_block_content = [&](bool explicit_block) -> UPtrExpr {
    auto expr = peek().text == "var" ? parse_variable() : parse_expr();

    if (explicit_block || peek().text == ";") {
      vector<UPtrExpr> exprs;
      exprs.push_back(std::move(expr));

      while (true) {
        if (peek().text == ";") {
          consume(nullopt);
          if (peek().type == Kind::END || peek().text == "}") {
            exprs.emplace_back(make_unique<Literal>(monostate()));
          } else {
            exprs.emplace_back(parse_expr());
          }
          continue;
        }

        if (peek().text == "{") {
          auto expr = peek().text == "var" ? parse_variable() : parse_expr();
          exprs.push_back(std::move(expr));
          continue;
        }

        break;
      }

      // cout << *exprs[0] << "parse_block_content " << endl;
      return make_unique<Block>(std::move(exprs));
    }
    return expr;
  };

  auto parse_new_block = [&]() -> UPtrExpr {
    consume("{");

    if (peek(-1).text == "{" && peek().text == "}") {
      // println("Block was empty!");
      return make_unique<Block>(vector<UPtrExpr>());
    }

    auto block = parse_block_content(true);
    // cout << *block << endl;
    consume("}");
    return block;
  };

  auto parse_loop = [&]() {
    consume("while");
    auto cond = parse_expr();
    consume("do");
    return make_unique<While>(std::move(cond), parse_expr());
  };

  parse_factor = [&]() -> UPtrExpr {
    auto token = peek();
    if (token.text == "(")
      return parse_parenthesized();

    if (token.text == "{")
      return parse_new_block();

    if (token.text == "while")
      return parse_loop();

    if (token.type == Kind::OPERATOR)
      return parse_unary();

    if (token.type == Kind::INT_LITERAL)
      return parse_int_literal();

    if (token.type == Kind::IDENTIFIER)
      return parse_identifier();

    if (token.type == Kind::CONDITIONAL)
      return parse_conditional();

    throw SyntaxError("Expected to find a suitable term, instead got: " +
                      token.text);
  };

  function<UPtrExpr(size_t)> parse_prec_level = [&](size_t level) {
    if (level >= la_binary_ops.size())
      return parse_factor();

    auto left = parse_prec_level(level + 1);
    while (std::ranges::contains(la_binary_ops[level], peek().text)) {
      const auto token = consume(nullopt);
      left = make_unique<BinaryOp>(std::move(left), token.text,
                                   parse_prec_level(level + 1));
    }
    return left;
  };

  parse_expr = [&]() {
    auto left = parse_prec_level(0);

    while (peek().text.contains("="sv)) {
      const auto token = consume(nullopt);

      left = make_unique<BinaryOp>(std::move(left), token.text, parse_expr());
    }

    verify_syntax();
    return left;
  };

  return parse_block_content(false);
}
} // namespace compiler
