#include <defs.h>

#include "syntax.h"

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
		auto type = g_lexer->eat_expect_keyword_declaration();

		check(type, "Expected a type, got '{}'", type->value);

		int type_indirection = 0;

		while (g_lexer->eat_if_current_is(Token_Indirection))
			++type_indirection;

		auto id = g_lexer->eat_expect(Token_Id);

		check(id, "Expected an id, got '{}'", id->value);

		const auto& id_name = id->value;

		auto ast_type = type->to_ast_type(type_indirection);
		
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
		else if (const bool global_var_decl = (next->id == Token_Semicolon); global_var_decl || next->id == Token_Assign)
		{
			add_id_type(id, type);

			ast->global_decls.push_back(_ALLOC(ast::ExprDecl, id_name, ast_type, global_var_decl ? nullptr : parse_expression(), true));

			if (g_lexer->is_current(Token_Semicolon))
				g_lexer->eat();
		}
	}
}

void Syntax::add_id_type(Token* id, Token* type)
{
	p_ctx.id_types.insert({ id->value, type->to_ast_type() });
}

std::vector<ast::Expr*> Syntax::parse_prototype_params_decl()
{
	std::vector<ast::Expr*> stmts;

	while (!g_lexer->eof())
	{
		auto type = g_lexer->eat_if_current_is_type();
		if (!type)
			break;

		int type_indirection = 0;

		while (g_lexer->eat_if_current_is(Token_Indirection))
			++type_indirection;

		auto id = g_lexer->eat_if_current_is(Token_Id);
		if (!id)
			break;

		const auto& id_name = id->value;

		add_id_type(id, type);

		stmts.push_back(_ALLOC(ast::ExprDecl, id_name, type->to_ast_type(type_indirection)));

		if (!g_lexer->is_current(Token_Comma))
			break;

		g_lexer->eat();
	}

	return stmts;
}

std::vector<ast::Expr*> Syntax::parse_call_params(ast::Prototype* prototype)
{
	std::vector<ast::Expr*> stmts;

	int param_index = 0;

	while (!g_lexer->eof() && !g_lexer->is_current(Token_ParenClose))
	{
		auto expr_param = parse_expression();
		if (!expr_param)
			break;

		if (auto param_decl = prototype->get_param(param_index))
		{
			/*if (auto casted_type = param_decl->type->normal_implicit_cast(expr_param->type))
				stmts.push_back(_ALLOC(ast::ExprCast, expr_param, casted_type));
			else */stmts.push_back(expr_param);

			++param_index;
		}

		if (!g_lexer->is_current(Token_Comma))
			break;

		g_lexer->eat();
	}

	return stmts;
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
				check(g_lexer->is_current(Token_Semicolon), "Missing token ';'");

				g_ctx.expect_semicolon = false;

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
	if (auto type = g_lexer->eat_if_current_is_type())
	{
		int type_indirection = 0;

		while (g_lexer->eat_if_current_is(Token_Indirection))
			++type_indirection;

		auto ast_type = type->to_ast_type(type_indirection);

		auto id = g_lexer->eat_if_current_is(Token_Id);

		check(id, "Expected an identifier, got '{}'", g_lexer->current()->value);

		add_id_type(id, type);

		auto expr_value = g_lexer->eat_if_current_is(Token_Assign) ? parse_expression() : nullptr;

		g_ctx.expect_semicolon = true;

		/*auto casted_type = expr_value ? id->normal_implicit_cast(expr_value->type) : nullptr;

		g_ctx.expect_semicolon = true;

		return (casted_type ? _ALLOC(ast::ExprDecl, id, type, _ALLOC(ast::ExprCast, expr_value, casted_type))
							: _ALLOC(ast::ExprDecl, id, type, expr_value));*/

		return _ALLOC(ast::ExprDecl, id->value, ast_type, expr_value);
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
		else if (type->id == Token_Do)
		{
			auto body = parse_body(nullptr);

			g_lexer->eat_expect(Token_While);
			g_lexer->eat_expect(Token_ParenOpen);

			auto condition = parse_expression(); g_lexer->eat_expect(Token_ParenClose);

			return _ALLOC(ast::StmtDoWhile, condition, body);
		}
		else if (type->id == Token_Break)		return _ALLOC(ast::StmtBreak);
		else if (type->id == Token_Continue)	return _ALLOC(ast::StmtContinue);
		else if (type->id == Token_Return)
		{
			auto ret = p_ctx.fn->type;
			auto expr_value = g_lexer->is_current(Token_Semicolon) ? nullptr : parse_expression();

			g_ctx.expect_semicolon = true;

			/*auto casted_type = ret->normal_implicit_cast(expr_value->type);

			g_ctx.expect_semicolon = true;

			return (casted_type ? _ALLOC(ast::StmtReturn, _ALLOC(ast::ExprCast, expr_value, casted_type), ret)
								: _ALLOC(ast::StmtReturn, expr_value, ret));*/

			return _ALLOC(ast::StmtReturn, expr_value, ret);
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

	while ((lookahead->flags & TypeFlag_Op) && lookahead->precedence <= min_precedence)
	{
		auto op = lookahead;

		g_lexer->eat();

		auto rhs = parse_primary_expression();

		lookahead = g_lexer->current();

		while ((lookahead->flags & TypeFlag_Op) && lookahead->precedence < op->precedence)
		{
			rhs = parse_expression_precedence(rhs, lookahead->precedence);
			lookahead = g_lexer->current();
		}

		/*if (auto casted_type = lhs->type->binary_implicit_cast(rhs->type))
		{
			if (lhs->type->id == casted_type->id)
				lhs = _ALLOC(ast::ExprBinaryOp, lhs, _ALLOC(ast::ExprCast, rhs, casted_type), op, lhs->type);
			else lhs = _ALLOC(ast::ExprBinaryOp, _ALLOC(ast::ExprCast, lhs, casted_type), rhs, op, rhs->type);
		}
		else */lhs = _ALLOC(ast::ExprBinaryOp, lhs, rhs, op->to_bin_op_type(), lhs->type);
	}

	return lhs;
}

ast::Expr* Syntax::parse_primary_expression()
{
	auto first = g_lexer->current();

	if (g_lexer->eat_if_current_is(Token_IntLiteral))
	{
		switch (first->size)
		{
		case 8:		first->id = (first->flags & TypeFlag_Unsigned) ? Token_U8  : Token_I8;  break;
		case 16:	first->id = (first->flags & TypeFlag_Unsigned) ? Token_U16 : Token_I16; break;
		case 32:	first->id = (first->flags & TypeFlag_Unsigned) ? Token_U32 : Token_I32; break;
		case 64:	first->id = (first->flags & TypeFlag_Unsigned) ? Token_U64 : Token_I64; break;
		}

		return _ALLOC(ast::ExprIntLiteral, first->value, first->to_ast_type());
	}
	else if (g_lexer->eat_if_current_is(Token_Sub) ||
			 g_lexer->eat_if_current_is(Token_Mul) ||
			 g_lexer->eat_if_current_is(Token_And) ||
			 g_lexer->eat_if_current_is(Token_Inc) ||
			 g_lexer->eat_if_current_is(Token_Dec) ||
			 g_lexer->eat_if_current_is(Token_LogicalNot))
	{
		return _ALLOC(ast::ExprUnaryOp, parse_primary_expression(), first->to_unary_op_type());
	}
	else if (auto id = g_lexer->eat_if_current_is(Token_Id))
	{
		const auto& id_name = id->value;

		auto id_type = get_id_type(id);

		auto curr = g_lexer->current();

		switch (const auto curr_token = g_lexer->current_token_id())
		{
		case Token_Assign:
		{
			g_lexer->eat();

			auto expr_value = parse_expression();

			/*auto casted_type = id->normal_implicit_cast(expr_value->type);

			return (casted_type ? _ALLOC(ast::ExprAssign, id, _ALLOC(ast::ExprCast, expr_value, casted_type))
								: _ALLOC(ast::ExprAssign, id, expr_value));*/

			return _ALLOC(ast::ExprAssign, id_name, id_type, expr_value);
		}
		case Token_Inc:
		case Token_Dec:
		{
			g_lexer->eat();

			return _ALLOC(ast::ExprUnaryOp, _ALLOC(ast::ExprId, id_name, id_type), curr->to_unary_op_type(), false);
		}
		case Token_AddAssign:
		case Token_SubAssign:
		case Token_MulAssign:
		case Token_DivAssign:
		case Token_XorAssign:
		{
			g_lexer->eat();

			auto expr_value = parse_expression();

			/*auto casted_type = id->normal_implicit_cast(expr_value->type);

			return (casted_type ? _ALLOC(ast::ExprBinaryOp, _ALLOC(ast::ExprId, id), _ALLOC(ast::ExprCast, expr_value, casted_type), id, casted_type)
								: _ALLOC(ast::ExprBinaryOp, _ALLOC(ast::ExprId, id), expr_value, id, expr_value->type));*/

			//return _ALLOC(ast::ExprBinaryOp, _ALLOC(ast::ExprId, id_name, id_type), expr_value, id, expr_value->type);
			return nullptr;
		}
		case Token_ParenOpen:
		{
			g_lexer->eat();

			auto called_prototype = g_ctx.get_prototype(id->value);
			auto call = _ALLOC(ast::ExprCall, called_prototype, id_name, called_prototype->type, false);	// todo - get if it's intrinsic/built-in or not

			call->stmts = parse_call_params(called_prototype);

			g_lexer->eat_expect(Token_ParenClose);

			return call;
		}
		}

		return _ALLOC(ast::ExprId, id_name, id_type);
	}
	else if (g_lexer->eat_if_current_is(Token_ParenOpen))
	{
		auto expr = parse_expression();

		check(expr, "Expected expression");
		check(g_lexer->eat_if_current_is(Token_ParenClose), "Expected ')', got '{}'", g_lexer->current()->value);

		return expr;
	}
	else if (auto static_val = g_lexer->eat_if_current_is_static_value())
		return _ALLOC(ast::ExprStaticValue, static_val->value, static_val->to_ast_type());

	return nullptr;
}

ast::Type Syntax::get_id_type(Token* id)
{
	auto it = p_ctx.id_types.find(id->value);
	return it != p_ctx.id_types.end() ? it->second : ast::Type {};
}