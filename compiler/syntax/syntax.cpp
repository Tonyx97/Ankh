#include <defs.h>

#include "syntax.h"

#include <intrin/intrin.h>

Syntax::Syntax()
{
	ast = _ALLOC(ast::AST);
}

Syntax::~Syntax()
{
	_FREE(ast);
}

void Syntax::print_ast()
{
	ast::Printer().print(ast);
}

void Syntax::run()
{
	while (!g_lexer->eof())
	{
		ast::Type ast_type {};

		parse_type(ast_type, true);

		auto id = g_lexer->eat_expect(Token_Id);

		check(id, "Expected an id");

		const auto& id_name = id->value;
		
		if (auto next = g_lexer->eat(); next->id == Token_ParenOpen)
		{
			auto prototype = _ALLOC(ast::Prototype, id_name, ast_type);

			p_ctx = prototype;

			prototype->params = parse_prototype_params_decl();

			if (auto paren_close = g_lexer->eat_expect(Token_ParenClose))
			{
				// we add the prototype here because a call inside this prototype
				// could make it recursive

				g_ctx.add_prototype(prototype);

				if (!g_lexer->eat_if_current_is(Token_Semicolon))
					prototype->body = parse_body(nullptr);

				ast->prototypes.push_back(prototype);

				if (auto previous_pt = g_ctx.get_prototype(id_name))
					if (previous_pt->is_decl())
						previous_pt->def = prototype;
			}
		}
	}
}

ast::StmtBody* Syntax::parse_body(ast::StmtBody* body)
{
	if (!g_lexer->eat_if_current_is(Token_BracketOpen))
		return nullptr;

	auto curr_body = _ALLOC(ast::StmtBody);

	while (!g_lexer->eof())
	{
		if (auto new_body = parse_body(curr_body))
			curr_body->stmts.push_back(new_body);

		if (auto stmt = parse_statement())
		{
			curr_body->stmts.push_back(stmt);

			if (g_ctx.expect_semicolon)
			{
				g_ctx.expect_semicolon = false;

				check(g_lexer->is_current(Token_Semicolon), "Missing token ';'");

				g_lexer->eat();
			}
			else if (g_lexer->is_current(Token_Semicolon))
				g_lexer->eat();
		}
		else break;
	}

	check(g_lexer->eat_expect(Token_BracketClose), "Expected a '}}', got '{}'", g_lexer->eof() ? "EOF" : g_lexer->current()->value);
	
	return curr_body;
}

ast::Base* Syntax::parse_statement()
{
	ast::Type ast_type {};

	if (auto type = parse_type(ast_type))
	{
		auto id = g_lexer->eat_if_current_is(Token_Id);

		check(id, "Expected an identifier, got '{}'", g_lexer->current()->value);

		auto expr_value = g_lexer->eat_if_current_is(Token_Assign) ? parse_expression() : nullptr;

		return return_and_expect_semicolon(_ALLOC(ast::ExprDecl, id->value, ast_type, expr_value));
	}
	else if (type = g_lexer->eat_if_current_is_keyword())
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

			auto init		= g_lexer->is_current(Token_Semicolon) ? nullptr : parse_statement();	g_lexer->eat_expect(Token_Semicolon);
			auto condition	= g_lexer->is_current(Token_Semicolon) ? nullptr : parse_expression();	g_lexer->eat_expect(Token_Semicolon);
			auto step		= g_lexer->is_current(Token_ParenClose) ? nullptr : parse_statement();	g_lexer->eat_expect(Token_ParenClose);

			return _ALLOC(ast::StmtFor, condition, init, step, parse_body(nullptr));
		}
		else if (type->id == Token_While)
		{
			g_lexer->eat_expect(Token_ParenOpen);

			auto condition = parse_expression(); g_lexer->eat_expect(Token_ParenClose);

			return _ALLOC(ast::StmtWhile, condition, parse_body(nullptr));
		}
		else if (type->id == Token_Do)
		{
			auto body = parse_body(nullptr);

			g_lexer->eat_expect(Token_While);
			g_lexer->eat_expect(Token_ParenOpen);

			auto condition = parse_expression(); g_lexer->eat_expect(Token_ParenClose);

			return _ALLOC(ast::StmtDoWhile, condition, body);
		}
		else if (type->id == Token_Break)		return return_and_expect_semicolon(_ALLOC(ast::StmtBreak));
		else if (type->id == Token_Continue)	return return_and_expect_semicolon(_ALLOC(ast::StmtContinue));
		else if (type->id == Token_Return)
		{
			auto& ret = p_ctx.fn->type;

			if (auto expr_value = g_lexer->is_current(Token_Semicolon) ? nullptr : parse_expression())
				return _ALLOC(ast::StmtReturn, expr_value);

			return return_and_expect_semicolon(_ALLOC(ast::StmtReturn, nullptr));
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

		while ((lookahead->flags & TokenFlag_Op) && lookahead->precedence < op->precedence)
		{
			rhs = parse_expression_precedence(rhs, lookahead->precedence);
			lookahead = g_lexer->current();
		}

		auto op_type = op->to_bin_op_type();

		check(rhs, "Expected expression");

		lhs = _ALLOC(ast::ExprBinaryOp, lhs, rhs, op_type, lhs->type);
	}

	return lhs;
}

ast::Expr* Syntax::parse_primary_expression()
{
	auto first = g_lexer->current();

	ast::Expr* ret_expr = nullptr;

	if (g_lexer->eat_if_current_is_int_literal())
		ret_expr = _ALLOC(ast::ExprIntLiteral, first->value, first->to_ast_type());
	else if (g_lexer->eat_if_current_is(Token_Sub) ||
			 g_lexer->eat_if_current_is(Token_Mul) ||
			 g_lexer->eat_if_current_is(Token_And) ||
			 g_lexer->eat_if_current_is(Token_Inc) ||
			 g_lexer->eat_if_current_is(Token_Dec) ||
			 g_lexer->eat_if_current_is(Token_LogicalNot))
	{
		auto rhs = parse_primary_expression();

		check(rhs, "Expected expression");

		ret_expr = _ALLOC(ast::ExprUnaryOp, rhs, first->to_unary_op_type());
	}
	else if (auto id = g_lexer->eat_if_current_is(Token_Id))
	{
		auto curr = g_lexer->current();

		switch (g_lexer->current_token_id())
		{
		case Token_Assign:
		{
			g_lexer->eat();

			auto expr_value = parse_expression();

			check(expr_value, "Expected expression");

			ret_expr = _ALLOC(ast::ExprAssign, id->value, expr_value);

			break;
		}
		case Token_Inc:
		case Token_Dec:
		{
			g_lexer->eat();

			ret_expr = _ALLOC(ast::ExprUnaryOp, _ALLOC(ast::ExprId, id->value), curr->to_unary_op_type(), false);

			break;
		}
		case Token_AddAssign:
		case Token_SubAssign:
		case Token_MulAssign:
		case Token_DivAssign:
		case Token_XorAssign:
		{
			g_lexer->eat();

			auto expr_value = parse_expression();

			check(expr_value, "Expected expression");

			auto op = curr->to_bin_op_type();

			ret_expr = _ALLOC(ast::ExprBinaryOp, _ALLOC(ast::ExprId, id->value), expr_value, op, expr_value->type);

			break;
		}
		case Token_ParenOpen:
		{
			g_lexer->eat();

			auto called_prototype = g_ctx.get_prototype(id->value);
			auto call = _ALLOC(ast::ExprCall, called_prototype, id->value, called_prototype->type, g_intrin->is_intrinsic(called_prototype->name));

			call->stmts = parse_call_params(called_prototype);

			g_lexer->eat_expect(Token_ParenClose);

			ret_expr = call;

			break;
		}
		default:
		{
			ret_expr = _ALLOC(ast::ExprId, id->value);
		}
		}
	}
	else if (g_lexer->eat_if_current_is(Token_ParenOpen))
	{
		ret_expr = parse_expression();

		check(ret_expr, "Expected expression");
		check(g_lexer->eat_if_current_is(Token_ParenClose), "Expected ')', got '{}'", g_lexer->current()->value);
	}

	auto curr = g_lexer->current();

	switch (auto token_id = g_lexer->current_token_id())
	{
	case Token_Dec:
	case Token_Inc:
	{
		g_lexer->eat();

		ret_expr = _ALLOC(ast::ExprUnaryOp, ret_expr, curr->to_unary_op_type(), false);

		break;
	}
	}

	return ret_expr;
}

Token* Syntax::parse_type(ast::Type& type_info, bool critical)
{
	auto type = critical ? g_lexer->eat_expect_keyword_declaration()
						 : g_lexer->eat_if_current_is_type();

	if (!type)
		return nullptr;

	int type_indirection = 0;

	while (g_lexer->eat_if_current_is(Token_Mul))
		++type_indirection;

	type_info = std::move(type->to_ast_type(type_indirection));

	return type;
}

std::vector<ast::Expr*> Syntax::parse_prototype_params_decl()
{
	std::vector<ast::Expr*> exprs;

	while (!g_lexer->eof())
	{
		ast::Type ast_type {};

		auto type = parse_type(ast_type);
		if (!type)
			break;

		auto id = g_lexer->eat_if_current_is(Token_Id);

		check(id, "Expected identifier");

		exprs.push_back(_ALLOC(ast::ExprDecl, id->value, ast_type));

		if (!g_lexer->is_current(Token_Comma))
			break;

		g_lexer->eat();
	}

	return exprs;
}

std::vector<ast::Expr*> Syntax::parse_call_params(ast::Prototype* prototype)
{
	std::vector<ast::Expr*> exprs;

	int param_index = 0;

	while (!g_lexer->eof() && !g_lexer->is_current(Token_ParenClose))
	{
		auto expr_param = parse_expression();
		if (!expr_param)
			break;

		if (auto param_decl = prototype->get_param(param_index))
		{
			exprs.push_back(expr_param);

			++param_index;
		}

		if (!g_lexer->is_current(Token_Comma))
			break;

		g_lexer->eat();
	}

	return exprs;
}