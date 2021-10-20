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

void Syntax::run()
{
	while (!g_lexer->eof())
	{
		if (auto type = g_lexer->eat_expect_keyword_declaration())
		{
			if (auto id = g_lexer->eat_expect(Token_Id))
			{
				if (auto next = g_lexer->eat(); next->id == Token_ParenOpen)
				{
					auto prototype = _ALLOC(ast::Prototype, id);

					prototype_ctx = prototype;

					prototype->params = parse_prototype_params_decl();
					prototype->ret_token = type;

					if (auto paren_close = g_lexer->eat_expect(Token_ParenClose))
					{
						if (!(prototype->body = parse_body(nullptr)))
							printf_s("[%s] SYNTAX ERROR: Error parsing prototype body\n", __FUNCTION__);

						tree->prototypes.push_back(prototype);
					}
				}
				else if (const bool global_var_decl = (next->id == Token_Semicolon); global_var_decl || next->id == Token_Assign)
				{
					add_id_type(id, type);

					tree->global_decls.push_back(_ALLOC(ast::ExprDecl, id, type, global_var_decl ? nullptr : parse_expression(), true));

					if (g_lexer->is_current(Token_Semicolon))
						g_lexer->eat();
				}
			}
			else printf_s("[%s] SYNTAX ERROR: Expected function id\n", __FUNCTION__);
		}
		else printf_s("[%s] SYNTAX ERROR: Expected a return type\n", __FUNCTION__);
	}
}

void Syntax::add_id_type(Token* id, Token* type)
{
	id->convert_to_type(type);

	prototype_ctx.id_types.insert({ id->value, type });
}

void Syntax::convert_id_to_type(Token* id)
{
	if (auto id_type = get_id_type(id); id_type && id->id == Token_Id)
		id->convert_to_type(id_type);
}

std::vector<ast::Base*> Syntax::parse_prototype_params_decl()
{
	std::vector<ast::Base*> stmts;

	while (!g_lexer->eof())
	{
		auto type = parse_type();
		if (!type)
			break;

		auto id = g_lexer->eat_if_current_is(Token_Id);
		if (!id)
			break;

		add_id_type(id, type);

		stmts.push_back(_ALLOC(ast::ExprDecl, id, type));

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
		if (auto id = g_lexer->eat_if_current_is(Token_Id))
		{
			add_id_type(id, type);

			auto expr_value = g_lexer->eat_if_current_is(Token_Assign) ? parse_expression() : nullptr;

			if (expr_value && !id->is_same_type(expr_value->get_token()))
				return _ALLOC(ast::ExprDecl, id, type, _ALLOC(ast::ExprCast, expr_value, Token_I32));
			else return _ALLOC(ast::ExprDecl, id, type, expr_value);
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
		else if (type->id == Token_While)
		{
			g_lexer->eat_expect(Token_ParenOpen);

			auto condition = parse_expression(); g_lexer->eat_expect(Token_ParenClose);

			return _ALLOC(ast::StmtWhile, condition, parse_body(nullptr));
		}
		else if (type->id == Token_Return)
			return _ALLOC(ast::StmtReturn, g_lexer->is_current(Token_Semicolon) ? nullptr : parse_expression(), prototype_ctx.fn->ret_token);
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

		while ((lookahead->flags & TokenFlag_Op) && lookahead->precedence < op->precedence)
		{
			rhs = parse_expression_precedence(rhs, lookahead->precedence);
			lookahead = g_lexer->current();
		}

		lhs = _ALLOC(ast::ExprBinaryOp, lhs, rhs, op);	// todo - add types to binary op here :o
	}

	return lhs;
}

ast::Expr* Syntax::parse_primary_expression()
{
	auto curr = g_lexer->current();

	if (g_lexer->eat_if_current_is(Token_IntLiteral))
	{
		// change the int literal type to the specific one

		switch (curr->size)
		{
		case 8:		curr->id = (curr->flags & TokenFlag_Unsigned) ? Token_U8  : Token_I8;  break;
		case 16:	curr->id = (curr->flags & TokenFlag_Unsigned) ? Token_U16 : Token_I16; break;
		case 32:	curr->id = (curr->flags & TokenFlag_Unsigned) ? Token_U32 : Token_I32; break;
		case 64:	curr->id = (curr->flags & TokenFlag_Unsigned) ? Token_U64 : Token_I64; break;
		}

		return _ALLOC(ast::ExprIntLiteral, curr);
	}
	else if (g_lexer->eat_if_current_is(Token_Sub) ||
			 g_lexer->eat_if_current_is(Token_Mul) ||
			 g_lexer->eat_if_current_is(Token_And) ||
			 g_lexer->eat_if_current_is(Token_LogicalNot))
	{
		return _ALLOC(ast::ExprUnaryOp, parse_primary_expression(), curr);
	}
	else if (auto id = g_lexer->eat_if_current_is(Token_Id))
	{
		convert_id_to_type(id);

		switch (const auto curr_token = g_lexer->current_token_id())
		{
		case Token_Assign:
		{
			g_lexer->eat();

			return _ALLOC(ast::ExprAssign, id, parse_expression());
		}
		case Token_AddAssign:
		case Token_SubAssign:
		case Token_MulAssign:
		case Token_DivAssign:
		case Token_XorAssign:
		{
			g_lexer->eat();

			return _ALLOC(ast::ExprBinaryOp, _ALLOC(ast::ExprId, id), parse_expression(), id);
		}
		case Token_ParenOpen:
		{
			g_lexer->eat();

			auto call = _ALLOC(ast::ExprCall, id, prototype_ctx.fn->ret_token, false);	// todo - get if it's intrinsic/built-in or not

			call->stmts = parse_call_params();

			g_lexer->eat_expect(Token_ParenClose);

			return call;
		}
		}

		return _ALLOC(ast::ExprId, id);
	}
	else if (g_lexer->eat_if_current_is(Token_ParenOpen))
	{
		if (auto expr = parse_expression(); expr && g_lexer->eat_expect(Token_ParenClose))
			return expr;
	}

	return nullptr;
}

Token* Syntax::get_id_type(Token* id)
{
	auto it = prototype_ctx.id_types.find(id->value);
	return it != prototype_ctx.id_types.end() ? it->second : nullptr;
}

Token* Syntax::parse_type()
{
	return (g_lexer->is_token_keyword_type() ? g_lexer->eat() : nullptr);
}

Token* Syntax::parse_keyword()
{
	return (g_lexer->is_token_keyword() ? g_lexer->eat() : nullptr);
}