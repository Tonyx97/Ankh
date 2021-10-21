#include <defs.h>

#include <lexer/lexer.h>

#include "ast.h"

void ast::Printer::print(AST* tree)
{
	for (auto decl : tree->global_decls)	print_decl(decl);
	for (auto prototype : tree->prototypes) print_prototype(prototype);
}

void ast::Printer::print_prototype(Prototype* prototype)
{
	if (first_prototype_printed)
		PRINT_NL;

	PRINT_TABS(White, 0, "Prototype '{}'", prototype->id_token->value);

	if (!prototype->params.empty())
	{
		PRINT_TABS(White, 0, " | Arguments: ");

		print_vec<ExprDecl>(Green, prototype->params, ", ", [](ExprDecl* e)
		{
			return Lexer::STRIFY_TYPE(e->get_token()) + " " + e->get_name();
		});
	}

	if (prototype->is_decl())
	{
		PRINT_TABS_NL(White, 0, " (Decl)");
	}
	else
	{
		PRINT_NL;

		print_body(prototype->body);

		PRINT_TABS_NL(White, curr_level, "End");
	}

	first_prototype_printed = true;
}

void ast::Printer::print_body(StmtBody* body)
{
	++curr_level;

	PRINT_TABS_NL(Cyan, curr_level, "Body");

	for (auto stmt_base : body->stmts)
		print_stmt(stmt_base);

	PRINT_TABS_NL(Cyan, curr_level, "End");

	--curr_level;
}

void ast::Printer::print_stmt(Base* stmt)
{
	if (auto body = rtti::cast<StmtBody>(stmt))					print_body(body);
	else if (auto stmt_if = rtti::cast<StmtIf>(stmt))			print_if(stmt_if);
	else if (auto stmt_for = rtti::cast<StmtFor>(stmt))			print_for(stmt_for);
	else if (auto stmt_while = rtti::cast<StmtWhile>(stmt))		print_while(stmt_while);
	else if (auto stmt_return = rtti::cast<StmtReturn>(stmt))	print_return(stmt_return);
	else if (auto expr = rtti::cast<Expr>(stmt))				print_expr(expr);
}

void ast::Printer::print_if(StmtIf* stmt_if)
{
	++curr_level;

	PRINT_TABS_NL(Blue, curr_level, "If");

	print_expr(stmt_if->expr);
	print_body(stmt_if->if_body);

	for (auto else_if : stmt_if->ifs)
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

void ast::Printer::print_while(StmtWhile* stmt_while)
{
	++curr_level;

	PRINT_TABS_NL(Blue, curr_level, "While");

	print_expr(stmt_while->condition);
	print_body(stmt_while->body);

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

	if (auto int_literal = rtti::cast<ExprIntLiteral>(expr))		print_expr_int(int_literal);
	else if (auto static_val = rtti::cast<ExprStaticValue>(expr))	print_static_val(static_val);
	else if (auto id = rtti::cast<ExprId>(expr))					print_id(id);
	else if (auto decl = rtti::cast<ExprDecl>(expr))				print_decl(decl);
	else if (auto assign = rtti::cast<ExprAssign>(expr))			print_assign(assign);
	else if (auto binary_op = rtti::cast<ExprBinaryOp>(expr))		print_expr_binary_op(binary_op);
	else if (auto unary_op = rtti::cast<ExprUnaryOp>(expr))			print_expr_unary_op(unary_op);
	else if (auto call = rtti::cast<ExprCall>(expr))				print_expr_call(call);
	else if (auto cast = rtti::cast<ExprCast>(expr))				print_cast(cast);

	--curr_level;
}

void ast::Printer::print_decl(ExprDecl* decl)
{
	PRINT_TABS_NL(Yellow, curr_level, "{}ecl.{} ({}) '{}'", decl->global ? "Global d" : "D", decl->rhs ? " assignment" : "", Lexer::STRIFY_TYPE(decl->get_token()), decl->get_name());

	if (decl->rhs)
		print_expr(decl->rhs);
}

void ast::Printer::print_assign(ExprAssign* assign)
{
	PRINT_TABS_NL(Yellow, curr_level, "Assignment '{}'", assign->get_name());

	print_expr(assign->rhs);
}

void ast::Printer::print_expr_int(ExprIntLiteral* expr)
{
	if (expr->type->flags & TokenFlag_Unsigned)
	{
		switch (auto size = expr->type->size)
		{
		case 8:  PRINT_TABS_NL(Yellow, curr_level, "int (u{}) '{}'", size, expr->type->u8);  break;
		case 16: PRINT_TABS_NL(Yellow, curr_level, "int (u{}) '{}'", size, expr->type->u16); break;
		case 32: PRINT_TABS_NL(Yellow, curr_level, "int (u{}) '{}'", size, expr->type->u32); break;
		case 64: PRINT_TABS_NL(Yellow, curr_level, "int (u{}) '{}'", size, expr->type->u64); break;
		}
	}
	else
	{
		switch (auto size = expr->type->size)
		{
		case 8:  PRINT_TABS_NL(Yellow, curr_level, "int (i{}) '{}'", size, expr->type->i8);  break;
		case 16: PRINT_TABS_NL(Yellow, curr_level, "int (i{}) '{}'", size, expr->type->i16); break;
		case 32: PRINT_TABS_NL(Yellow, curr_level, "int (i{}) '{}'", size, expr->type->i32); break;
		case 64: PRINT_TABS_NL(Yellow, curr_level, "int (i{}) '{}'", size, expr->type->i64); break;
		}
	}
}

void ast::Printer::print_static_val(ExprStaticValue* expr)
{
	PRINT_TABS_NL(Yellow, curr_level, "{} '{}'", Lexer::STRIFY_TYPE(expr->type), expr->type->value);
}

void ast::Printer::print_id(ast::ExprId* expr)
{
	PRINT_TABS_NL(Yellow, curr_level, "Id ({}) '{}'", Lexer::STRIFY_TYPE(expr->id), expr->get_name());
}

void ast::Printer::print_expr_unary_op(ast::ExprUnaryOp* expr)
{
	PRINT_TABS_NL(Yellow, curr_level, "Unary Op ({})", expr->get_name());

	++curr_level;

	PRINT_TABS_NL(Yellow, curr_level, "Value '{}'", expr->rhs->get_name());

	if (auto value = rtti::cast<ExprBinaryOp>(expr->rhs))			print_expr_binary_op(value);
	else if (auto value_unary = rtti::cast<ExprUnaryOp>(expr->rhs))	print_expr_unary_op(value_unary);

	--curr_level;
}

void ast::Printer::print_expr_binary_op(ExprBinaryOp* expr)
{
	PRINT_TABS_NL(Yellow, curr_level, "Binary Op ({})", expr->get_name());

	++curr_level;

	if (expr->lhs)
	{
		PRINT_TABS_NL(Yellow, curr_level, "Left operand:");
		print_expr(expr->lhs);
	}

	if (expr->rhs)
	{
		PRINT_TABS_NL(Yellow, curr_level, "Right operand:");
		print_expr(expr->rhs);
	}

	--curr_level;
}

void ast::Printer::print_expr_call(ExprCall* expr)
{
	PRINT_TABS_NL(Yellow, curr_level, "Prototype Call ({})", expr->get_name());

	for (auto param : expr->stmts)
	{
		if (auto call = rtti::cast<ExprCall>(param))
		{
			++curr_level;

			print_expr_call(call);

			--curr_level;
		}
		else
		{
			PRINT_TABS_NL(Yellow, curr_level, "Param:");
			print_expr(param);
		}
	}
}

void ast::Printer::print_cast(ExprCast* expr)
{
	PRINT_TABS_NL(Yellow, curr_level, "{} cast {} {} to {}",
		expr->implicit ? "Implicit" : "Explicit",
		Lexer::STRIFY_TYPE(expr->rhs->type),
		expr->rhs->get_name(),
		Lexer::STRIFY_TYPE(expr->type));

	print_expr(expr->rhs);
}