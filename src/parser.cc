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
      throw SyntaxError("Parser: input was empty");

    if (pos + offset < static_cast<ssize_t>(tokens.size())) {
      return tokens.at(pos + offset);
    }

    throw runtime_error("Parser: tried to peek out of bounds");
  };

  auto consume = [&](optional<variant<string, vector<string_view>>> expected) {
    const auto &token = peek();

    if (expected.has_value()) {
      if (expected->index() == 0) {
        if (get<0>(*expected) != token.text) {
          throw SyntaxError("Parser: consume expected " + get<0>(*expected) +
                            " but found: " + token.as_string());
        }
      } else {
        if (!std::ranges::contains(get<1>(*expected), token.text))
          throw SyntaxError("Parser: consumed unexpected token: " + token.text);
      }
    }

    ++pos;
    return token;
  };

  auto parse_int_literal = [&]() {
    auto token = consume(nullopt);
    return make_unique<Literal>(stoul(token.text));
  };

  auto parse_function_call = [&](unique_ptr<Identifier> name) {
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
    auto token = consume(nullopt);

    if (peek().text == "(") {
      return parse_function_call(make_unique<Identifier>(token.text));
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

  auto parse_variable_decl = [&]() {
    consume("var");
    auto token = consume(nullopt);
    if (token.type != Kind::IDENTIFIER) {
      throw SyntaxError("Parser: Variable name must be an identifier, found: " +
                        token.as_string());
    }
    consume("=");
    return make_unique<Variable>(token.text, parse_expr());
  };

  auto parse_top_level = [&]() -> UPtrExpr {
    // Only allow variable declarations at the top level
    return peek().text == "var" ? parse_variable_decl() : parse_expr();
  };

  auto parse_block = [&]() -> UPtrExpr {
    consume("{");

    // Case empty block
    if (peek().text == "}") {
      consume("}");
      return make_unique<Block>(vector<UPtrExpr>());
    }

    auto top_level_exprs = std::vector<UPtrExpr>{};
    top_level_exprs.emplace_back(parse_top_level());

    // If expr in a block ends in ;
    // OR expr was a block that did not end in ;
    // There will be more expressions in current scope
    while (peek().text == ";" || peek(-1).text == "}") {

      if (peek().text == ";") {
        // Case 1: Consume and parse the next expression
        // unless the next token is scope end or EOF -> return unit
        consume(";");
        if (peek().text == "}" || peek().type == Kind::END) {
          top_level_exprs.emplace_back(make_unique<Literal>(std::monostate()));
        } else {
          top_level_exprs.emplace_back(parse_top_level());
        }
      } else if (peek().text != "}" && peek().type != Kind::END) {
        // Case 2: previous token was a scope end
        // Unless text token is } or EOF there will be more to parse
        top_level_exprs.emplace_back(parse_top_level());
      } else {
        break;
      }
    }

    consume("}");

    return make_unique<Block>(std::move(top_level_exprs));
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
      return parse_block();

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

    throw SyntaxError(std::format(
        "Parser: Expected to find a suitable term, instead got: {} at pos {}",
        token.text, pos));
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

    return left;
  };

  // Entrypoint
  auto result = parse_block();
  if (peek().type != Kind::END) {
    throw SyntaxError("Parser: trailing junk in input");
  }

  return result;
}
} // namespace compiler
