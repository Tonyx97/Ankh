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
		if (auto prototype = parse_prototype())
			ast->prototypes.push_back(prototype);
	}
}

ast::Prototype* Syntax::parse_prototype()
{
	auto type = parse_type(true);
	auto id = g_lexer->eat_expect(Token_Id);

	check(id, "Expected an id");

	const auto& id_name = id->value;

	check(g_lexer->eat_expect(Token_ParenOpen), "Expected '('");

	auto prototype = _ALLOC(ast::Prototype, id_name, type.value());

	prototype->params = parse_prototype_params_decl();

	g_lexer->eat_expect(Token_ParenClose);

	// we add the prototype here because a call inside this prototype
	// could make it recursive

	auto prev_prototype = g_ctx.get_prototype(id_name);
	if (!prev_prototype)
	{
		g_ctx.add_prototype(prototype);
	}
	else
	{
		check(prototype->type == prev_prototype->type, "Function {} has mismatched return types.", prototype->name);
		check(prototype->params.size() == prev_prototype->params.size(), "Function {} has different number of parameters.", prototype->name);

		for (int i = 0; i < prototype->params.size(); ++i) 
		{
			check(prototype->params[i]->type == prev_prototype->params[i]->type,
				"Function {} has mismatched parameters.", prototype->name);
		}

		_FREE(prototype);

		prototype = prev_prototype;
	}

	if (!g_lexer->eat_if_current_is(Token_Semicolon))
	{
		check(!prototype->body, "Function {} already has body.", prototype->name);
		prototype->body = parse_body(nullptr);
	}

	return prev_prototype ? nullptr : prototype;
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
	if (auto type = parse_type())
	{
		auto id = g_lexer->eat_if_current_is(Token_Id);

		check(id, "Expected an identifier, got '{}'", g_lexer->current()->value);

		auto expr_value = g_lexer->eat_if_current_is(Token_Assign) ? parse_expression() : nullptr;

		return return_and_expect_semicolon(_ALLOC(ast::ExprDecl, id->value, type.value(), expr_value));
	}
	else if (auto curr_type = g_lexer->eat_if_current_is_keyword())
	{
		switch (curr_type->id)
		{
		case Token_If:
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
		case Token_For:
		{
			g_lexer->eat_expect(Token_ParenOpen);

			auto init = g_lexer->is_current(Token_Semicolon) ? nullptr : parse_statement();	g_lexer->eat_expect(Token_Semicolon);
			auto condition = g_lexer->is_current(Token_Semicolon) ? nullptr : parse_expression();	g_lexer->eat_expect(Token_Semicolon);
			auto step = g_lexer->is_current(Token_ParenClose) ? nullptr : parse_statement();	g_lexer->eat_expect(Token_ParenClose);

			return _ALLOC(ast::StmtFor, condition, init, step, parse_body(nullptr));
		}
		case Token_While:
		{
			g_lexer->eat_expect(Token_ParenOpen);

			auto condition = parse_expression(); g_lexer->eat_expect(Token_ParenClose);

			return _ALLOC(ast::StmtWhile, condition, parse_body(nullptr));
		}
		case Token_Do:
		{
			auto body = parse_body(nullptr);

			g_lexer->eat_expect(Token_While);
			g_lexer->eat_expect(Token_ParenOpen);

			auto condition = parse_expression(); g_lexer->eat_expect(Token_ParenClose);

			return _ALLOC(ast::StmtDoWhile, condition, body);
		}
		case Token_Break:		return return_and_expect_semicolon(_ALLOC(ast::StmtBreak));
		case Token_Continue:	return return_and_expect_semicolon(_ALLOC(ast::StmtContinue));
		case Token_Return:
		{
			if (auto expr_value = g_lexer->is_current(Token_Semicolon) ? nullptr : parse_expression())
				return _ALLOC(ast::StmtReturn, expr_value);

			return return_and_expect_semicolon(_ALLOC(ast::StmtReturn, nullptr));
		}
		}
	}

	return return_and_expect_semicolon(parse_expression());
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

		check(rhs, "Expected expression");

		if (op->flags & TokenFlag_Assignation)
		{
			TokenID new_operator = Token_None;

			switch (op->id)
			{
			case Token_ShrAssign: new_operator = Token_Shr; break;
			case Token_ShlAssign: new_operator = Token_Shl; break;
			case Token_AddAssign: new_operator = Token_Add; break;
			case Token_SubAssign: new_operator = Token_Sub; break;
			case Token_MulAssign: new_operator = Token_Mul; break;
			case Token_ModAssign: new_operator = Token_Mod; break;
			case Token_DivAssign: new_operator = Token_Div; break;
			case Token_AndAssign: new_operator = Token_And; break;
			case Token_OrAssign: new_operator = Token_Or; break;
			case Token_XorAssign: new_operator = Token_Xor; break;
			}

			if (new_operator != Token_None)
			{
				lhs = _ALLOC(ast::ExprBinaryAssign, lhs, rhs, Token::to_bin_op_type(new_operator));
			}
			else
			{
				lhs = _ALLOC(ast::ExprAssign, lhs, rhs);
			}
		}
		else
		{
			auto op_type = op->to_bin_op_type();
			lhs = _ALLOC(ast::ExprBinaryOp, lhs, rhs, op_type, lhs->type);
		}
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
		if (g_lexer->current_token_id() == Token_ParenOpen) 
		{
			g_lexer->eat();

			auto call = _ALLOC(ast::ExprCall, id->value, g_intrin->is_intrinsic(id->value));

			call->exprs = parse_call_params();

			g_lexer->eat_expect(Token_ParenClose);

			ret_expr = call;
		}
		else
		{
			ret_expr = _ALLOC(ast::ExprId, id->value);
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

ast::TypeOpt Syntax::parse_type(bool expect)
{
	auto token = expect ? g_lexer->eat_expect_keyword_declaration()
						: g_lexer->eat_if_current_is_type();

	if (!token)
		return {};

	int type_indirection = 0;

	while (g_lexer->eat_if_current_is(Token_Mul))
		++type_indirection;

	return token->to_ast_type(type_indirection);
}

std::vector<ast::Expr*> Syntax::parse_prototype_params_decl()
{
	std::vector<ast::Expr*> exprs;

	while (!g_lexer->eof())
	{
		auto type = parse_type();
		if (!type)
			break;

		auto id = g_lexer->eat_if_current_is(Token_Id);

		check(id, "Expected identifier");

		exprs.push_back(_ALLOC(ast::ExprDecl, id->value, type.value()));

		if (!g_lexer->is_current(Token_Comma))
			break;

		g_lexer->eat();
	}

	return exprs;
}

std::vector<ast::Expr*> Syntax::parse_call_params()
{
	std::vector<ast::Expr*> exprs;

	int param_index = 0;

	while (!g_lexer->eof() && !g_lexer->is_current(Token_ParenClose))
	{
		auto expr_param = parse_expression();
		if (!expr_param)
			break;

		exprs.push_back(expr_param);

		if (!g_lexer->is_current(Token_Comma))
			break;

		g_lexer->eat();
	}

	return exprs;
}