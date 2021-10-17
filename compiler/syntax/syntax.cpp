#include <defs.h>

#include "syntax.h"

Syntax::Syntax()
{
	tree = _ALLOC(ast::AST);
}

Syntax::~Syntax()
{
	_FREE(tree);
}

void Syntax::print_ast()
{
	ast::Printer().print(tree);
}

bool Syntax::run()
{
	while (!g_lexer->eof())
	{
		if (auto prototype = parse_prototype())
			tree->prototypes.push_back(prototype);
	}

	return true;
}

ast::Prototype* Syntax::parse_prototype()
{
	if (auto ret_token = g_lexer->eat_expect_keyword_declaration())
	{
		if (auto id = g_lexer->eat_expect(Token_Id))
		{
			if (auto paren_open = g_lexer->eat_expect(Token_ParenOpen))
			{
				auto prototype = _ALLOC(ast::Prototype, id->value);

				prototype->params = parse_prototype_params_decl();
				prototype->ret_token = ret_token;

				if (auto paren_close = g_lexer->eat_expect(Token_ParenClose))
				{
					if (!g_lexer->is_current(Token_Semicolon))
					{
						if (prototype->body = parse_body(nullptr))
							return prototype;
						else printf_s("Failed parsing main prototype body\n");
					}
					else
					{
						g_lexer->eat();

						return prototype;
					}
				}

				_FREE(prototype);
			}
		}
		else printf_s("[%s] SYNTAX ERROR: Expected function id\n", __FUNCTION__);
	}
	else printf_s("[%s] SYNTAX ERROR: Expected a return type\n", __FUNCTION__);

	return nullptr;
}

std::vector<ast::Base*> Syntax::parse_prototype_params_decl()
{
	std::vector<ast::Base*> stmts;

	while (!g_lexer->eof())
	{
		auto param_type = parse_type();
		if (!param_type)
			break;

		auto param_id = parse_id();
		if (!param_id)
			break;

		stmts.push_back(_ALLOC(ast::ExprDeclOrAssign, param_id->value, nullptr, param_type));

		if (!g_lexer->is_current(Token_Comma))
			break;

		g_lexer->eat();
	}

	return stmts;
}

std::vector<ast::Expr*> Syntax::parse_call_params()
{
	std::vector<ast::Expr*> stmts;

	while (!g_lexer->eof() && !g_lexer->is_current(Token_ParenClose))
	{
		auto param_value = parse_expression();
		if (!param_value)
			break;

		stmts.push_back(param_value);

		if (!g_lexer->is_current(Token_Comma))
			break;

		g_lexer->eat();
	}

	return stmts;
}

ast::StmtBody* Syntax::parse_body(ast::StmtBody* body)
{
	if (!g_lexer->is_current(Token_BracketOpen))
		return nullptr;

	g_lexer->eat();

	auto curr_body = _ALLOC(ast::StmtBody);

	while (!g_lexer->eof())
	{
		if (auto new_body = parse_body(curr_body))
			curr_body->stmts.push_back(new_body);

		if (auto stmt = parse_statement())
		{
			curr_body->stmts.push_back(stmt);

			if (g_lexer->is_current(Token_Semicolon))
				g_lexer->eat();
		}
		else break;
	}

	if (g_lexer->eat_expect(Token_BracketClose))
		return curr_body;
	
	//parser_error("Expected a '}', got '%s'", g_lexer->eof() ? "EOF" : g_lexer->current_value().c_str());
	
	return nullptr;
}

ast::Base* Syntax::parse_statement()
{
	if (auto type = parse_type())
	{
		if (auto id = parse_id())
		{
			if (g_lexer->is_current(Token_Assign))
			{
				g_lexer->eat();
				return _ALLOC(ast::ExprDeclOrAssign, id->value, parse_expression(), type);
			}

			return _ALLOC(ast::ExprDeclOrAssign, id->value, nullptr, type);
		}
		else printf_s("[%s] SYNTAX ERROR: Expected an identifier\n", __FUNCTION__);
	}
	else if (type = parse_keyword())
	{
		if (type->id == Token_If)
		{
			g_lexer->eat_expect(Token_ParenOpen);

			auto if_expr = parse_expression();

			g_lexer->eat_expect(Token_ParenClose);

			auto if_stmt = _ALLOC(ast::StmtIf, if_expr, parse_body(nullptr));

			if (g_lexer->is_current(Token_Else))
			{
				if (g_lexer->is_next(Token_If))
				{
					do {
						g_lexer->eat();
						g_lexer->eat();

						g_lexer->eat_expect(Token_ParenOpen);

						auto else_if_expr = parse_expression();

						g_lexer->eat_expect(Token_ParenClose);

						if_stmt->ifs.push_back(_ALLOC(ast::StmtIf, else_if_expr, parse_body(nullptr)));

					} while (!g_lexer->eof() && g_lexer->is_current(Token_Else) && g_lexer->is_next(Token_If));
				}

				if (g_lexer->is_current(Token_Else))
				{
					g_lexer->eat();

					if_stmt->else_body = parse_body(nullptr);
				}
			}

			return if_stmt;
		}
		else if (type->id == Token_For)
		{
			g_lexer->eat_expect(Token_ParenOpen);
			
			auto init		= g_lexer->is_current(Token_Semicolon)	? nullptr : parse_statement();	g_lexer->eat_expect(Token_Semicolon);
			auto condition	= g_lexer->is_current(Token_Semicolon)	? nullptr : parse_expression();	g_lexer->eat_expect(Token_Semicolon);
			auto step		= g_lexer->is_current(Token_ParenClose) ? nullptr : parse_statement();	g_lexer->eat_expect(Token_ParenClose);

			return _ALLOC(ast::StmtFor, condition, init, step, parse_body(nullptr));
		}
		else if (type->id == Token_Return)
		{
			return _ALLOC(ast::StmtReturn, g_lexer->is_current(Token_Semicolon) ? nullptr : parse_expression());
		}
	}
	
	return parse_expression();
}

ast::Expr* Syntax::parse_expression()
{
	return parse_expression_precedence(parse_primary_expression());
}

ast::Expr* Syntax::parse_expression_precedence(ast::Expr* lhs, int min_precedence)
{
	if (!lhs)
		return nullptr;

	auto lookahead = g_lexer->current();

	while ((lookahead->flags & TokenFlag_Op) && lookahead->precedence <= min_precedence)
	{
		auto op = lookahead;

		g_lexer->eat();

		auto rhs = parse_primary_expression();

		lookahead = g_lexer->current();

		// no right associative operators yet

		if (lookahead->id == Token_ParenClose)
			g_lexer->eat();

		while ((lookahead->flags & TokenFlag_Op) && lookahead->precedence < op->precedence)
		{
			rhs = parse_expression_precedence(rhs, lookahead->precedence);
			lookahead = g_lexer->current();
		}

		lhs = _ALLOC(ast::ExprBinaryOp, lhs, rhs, op);
	}

	return lhs;
}

ast::Expr* Syntax::parse_primary_expression()
{
	auto curr = g_lexer->current();

	if (g_lexer->is_current(Token_IntLiteral))
	{
		g_lexer->eat();

		return _ALLOC(ast::ExprIntLiteral, curr);
	}
	else if (g_lexer->is_current(Token_Sub) ||
			 g_lexer->is_current(Token_Mul) ||
			 g_lexer->is_current(Token_And) ||
			 g_lexer->is_current(Token_LogicalNot))
	{
		g_lexer->eat();

		return _ALLOC(ast::ExprUnaryOp, parse_primary_expression(), curr);
	}
	else if (g_lexer->is_current(Token_Id))
	{
		auto id = g_lexer->eat();

		switch (const auto curr_token = g_lexer->current_token_id())
		{
		case Token_Assign:
		{
			g_lexer->eat();

			return _ALLOC(ast::ExprDeclOrAssign, id->value, parse_expression());
		}
		case Token_AddAssign:
		case Token_SubAssign:
		case Token_MulAssign:
		case Token_DivAssign:
		case Token_XorAssign:
		{
			g_lexer->eat();

			return _ALLOC(ast::ExprBinaryOp, _ALLOC(ast::ExprId, curr->value), parse_expression(), curr);	//
		}
		case Token_ParenOpen:
		{
			g_lexer->eat();

			auto call = _ALLOC(ast::ExprCall, curr->value, false);	// todo

			call->stmts = parse_call_params();

			g_lexer->eat_expect(Token_ParenClose);

			return call;
		}
		}

		return _ALLOC(ast::ExprId, curr->value);
	}
	else if (g_lexer->is_current(Token_ParenOpen))
	{
		g_lexer->eat();

		return parse_expression();
	}
	else if (g_lexer->is_current(Token_ParenClose))
	{
		g_lexer->eat();

		//return parse_expression();
	}

	return nullptr;
}

Token* Syntax::parse_type()
{
	return (g_lexer->is_token_keyword_type() ? g_lexer->eat() : nullptr);
}

Token* Syntax::parse_keyword()
{
	return (g_lexer->is_token_keyword() ? g_lexer->eat() : nullptr);
}

Token* Syntax::parse_id()
{
	return (g_lexer->is_current(Token_Id) ? g_lexer->eat() : nullptr);
}