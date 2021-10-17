#include <defs.h>

#include <lexer/lexer.h>

#include "ast.h"

void ast::Printer::print(AST* tree)
{
	for (auto prototype : tree->prototypes)
		print_prototype(prototype);
}

void ast::Printer::print_prototype(Prototype* prototype)
{
	if (first_prototype_printed)
		PRINT_NL;

	PRINT_TABS(White, 0, "Prototype '{}'", prototype->name);

	if (!prototype->params.empty())
	{
		PRINT_TABS(White, 0, " | Arguments: ");

		print_vec<ExprDeclOrAssign>(Green, prototype->params, ", ", [](ExprDeclOrAssign* e)
		{
			return Lexer::STRIFY_TYPE(e->token) + " " + e->name;
		});
	}

	const bool declaration = prototype->is_declaration();

	if (declaration)
		PRINT_TABS(White, 0, " (Declaration)");
	else PRINT_NL;

	if (prototype->body)
		print_body(prototype->body);

	if (!declaration)
		PRINT_TABS_NL(White, curr_level, "End");

	first_prototype_printed = true;
}

void ast::Printer::print_body(StmtBody* body)
{
	++curr_level;

	PRINT_TABS_NL(Cyan, curr_level, "Body");

	for (auto&& stmt_base : body->stmts)
		print_stmt(stmt_base);

	PRINT_TABS_NL(Cyan, curr_level, "End");

	--curr_level;
}

void ast::Printer::print_stmt(Base* stmt)
{
	if (auto body = rtti::cast<StmtBody>(stmt))
		print_body(body);
	else if (auto stmt_if = rtti::cast<StmtIf>(stmt))
		print_if(stmt_if);
	else if (auto stmt_for = rtti::cast<StmtFor>(stmt))
		print_for(stmt_for);
	else if (auto stmt_return = rtti::cast<StmtReturn>(stmt))
		print_return(stmt_return);
	else if (auto expr = rtti::cast<Expr>(stmt))
		print_expr(expr);
}

void ast::Printer::print_if(StmtIf* stmt_if)
{
	++curr_level;

	PRINT_TABS_NL(Blue, curr_level, "If");

	print_expr(stmt_if->expr);

	print_body(stmt_if->if_body);

	for (auto&& else_if : stmt_if->ifs)
	{
		PRINT_TABS_NL(Blue, curr_level, "Else If");

		print_expr(else_if->expr);

		print_body(else_if->if_body);
	}

	if (stmt_if->else_body)
	{
		PRINT_TABS_NL(Blue, curr_level, "Else");

		print_body(stmt_if->else_body);
	}

	--curr_level;
}

void ast::Printer::print_for(ast::StmtFor* stmt_for)
{
	++curr_level;

	PRINT_TABS_NL(Blue, curr_level, "For");

	print_stmt(stmt_for->init);
	print_expr(stmt_for->condition);
	print_stmt(stmt_for->step);
	print_body(stmt_for->body);

	--curr_level;
}

void ast::Printer::print_return(ast::StmtReturn* stmt_return)
{
	PRINT_TABS_NL(Blue, curr_level, "Return");

	print_expr(stmt_return->expr);
}

void ast::Printer::print_expr(Expr* expr)
{
	++curr_level;

	if (auto int_literal = rtti::cast<ExprIntLiteral>(expr))
		print_expr_int(int_literal);
	else if (auto id = rtti::cast<ExprId>(expr))
		print_id(id);
	else if (auto decl_or_assign = rtti::cast<ExprDeclOrAssign>(expr))
		print_decl_or_assign(decl_or_assign);
	else if (auto binary_op = rtti::cast<ExprBinaryOp>(expr))
		print_expr_binary_op(binary_op);
	else if (auto unary_op = rtti::cast<ExprUnaryOp>(expr))
		print_expr_unary_op(unary_op);
	else if (auto call = rtti::cast<ExprCall>(expr))
		print_expr_call(call);

	--curr_level;
}

void ast::Printer::print_decl_or_assign(ExprDeclOrAssign* assign)
{
	if (assign->token)
	{
		PRINT_TABS_NL(Yellow, curr_level, "Declaration assignment '{}' ({})", assign->name, Lexer::STRIFY_TYPE(assign->token));
	}
	else
	{
		if (assign->value)
		{
			PRINT_TABS_NL(Yellow, curr_level, "Assignment '{}'", assign->name);
		}
		else
		{
			PRINT_TABS_NL(Yellow, curr_level, "Declaration '{}'", assign->name);
		}
	}

	if (assign->value)
		print_expr(assign->value);
}

void ast::Printer::print_expr_int(ExprIntLiteral* expr)
{
	switch (expr->token->id)
	{
	case Token_U8:  PRINT_TABS_NL(Yellow, curr_level, "Expr '{}' u8", expr->value.u8);   break;
	case Token_U16: PRINT_TABS_NL(Yellow, curr_level, "Expr '{}' u16", expr->value.u16); break;
	case Token_U32: PRINT_TABS_NL(Yellow, curr_level, "Expr '{}' u32", expr->value.u32); break;
	case Token_U64: PRINT_TABS_NL(Yellow, curr_level, "Expr '{}' u64", expr->value.u64); break;
	case Token_I8:  PRINT_TABS_NL(Yellow, curr_level, "Expr '{}' i8", expr->value.i8);   break;
	case Token_I16: PRINT_TABS_NL(Yellow, curr_level, "Expr '{}' i16", expr->value.i16); break;
	case Token_I32: PRINT_TABS_NL(Yellow, curr_level, "Expr '{}' i32", expr->value.i32); break;
	case Token_I64: PRINT_TABS_NL(Yellow, curr_level, "Expr '{}' i64", expr->value.i64); break;
	}
}

void ast::Printer::print_id(ast::ExprId* expr)
{
	PRINT_TABS_NL(Yellow, curr_level, "Id '{}'", expr->name);
}

void ast::Printer::print_expr_unary_op(ast::ExprUnaryOp* expr)
{
	PRINT_TABS_NL(Yellow, curr_level, "Unary Op ({})", expr->get_name());

	++curr_level;

	PRINT_TABS_NL(Yellow, curr_level, "Value '{}'", expr->value->get_name());

	if (auto value = rtti::cast<ExprBinaryOp>(expr->value))
		print_expr_binary_op(value);
	else if (auto value_unary = rtti::cast<ExprUnaryOp>(expr->value))
		print_expr_unary_op(value_unary);

	--curr_level;
}

void ast::Printer::print_expr_binary_op(ExprBinaryOp* expr)
{
	PRINT_TABS_NL(Yellow, curr_level, "Binary Op ({})", expr->get_name());

	++curr_level;

	if (expr->left)
		PRINT_TABS_NL(Yellow, curr_level, "Left operand '{}'", expr->left->get_name());

	if (auto left = rtti::cast<ExprBinaryOp>(expr->left))
		print_expr_binary_op(left);
	else if (auto left_unary = rtti::cast<ExprUnaryOp>(expr->left))
		print_expr_unary_op(left_unary);

	if (expr->right)
		PRINT_TABS_NL(Yellow, curr_level, "Right operand '{}'", expr->right->get_name());

	if (auto right = rtti::cast<ExprBinaryOp>(expr->right))
		print_expr_binary_op(right);
	else if (auto right_unary = rtti::cast<ExprUnaryOp>(expr->right))
		print_expr_unary_op(right_unary);

	--curr_level;
}

void ast::Printer::print_expr_call(ExprCall* expr)
{
	PRINT_TABS_NL(Yellow, curr_level, "Prototype Call ({})", expr->name);

	for (auto&& param : expr->stmts)
	{
		if (auto call = rtti::cast<ExprCall>(param))
		{
			++curr_level;

			print_expr_call(call);

			--curr_level;
		}
		else print_expr(param);
	}
}