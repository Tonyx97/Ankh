#include <defs.h>

#include <lexer/lexer.h>

#include "ast.h"

void ast::Printer::print(AST* ast)
{
	PRINT_NL;

	for (auto prototype : ast->prototypes) print_prototype(prototype);
}

void ast::Printer::print_prototype(Prototype* prototype)
{
	if (first_prototype_printed)
		PRINT_NL;

	PRINT_TABS(White, 0, "prototype '{}'", prototype->name);

	if (!prototype->params.empty())
	{
		PRINT_TABS(White, 0, " | arguments: ");

		print_vec<ExprDecl>(Green, prototype->params, ", ", [](ExprDecl* e)
		{
			return e->type.str() + " " + e->name;
		});
	}

	if (prototype->is_decl())
	{
		PRINT_TABS_NL(White, 0, " (decl)");
	}
	else
	{
		PRINT_NL;

		print_body(prototype->body);

		PRINT_TABS_NL(White, curr_level, "end");
	}

	first_prototype_printed = true;
}

void ast::Printer::print_body(StmtBody* body)
{
	++curr_level;

	PRINT_TABS_NL(Cyan, curr_level, "body");

	for (auto stmt_base : body->stmts)
		print_stmt(stmt_base);

	PRINT_TABS_NL(Cyan, curr_level, "end");

	--curr_level;
}

void ast::Printer::print_stmt(Base* stmt)
{
	if (auto body = rtti::cast<StmtBody>(stmt))						print_body(body);
	else if (auto stmt_if = rtti::cast<StmtIf>(stmt))				print_if(stmt_if);
	else if (auto stmt_for = rtti::cast<StmtFor>(stmt))				print_for(stmt_for);
	else if (auto stmt_while = rtti::cast<StmtWhile>(stmt))			print_while(stmt_while);
	else if (auto stmt_do_while = rtti::cast<StmtDoWhile>(stmt))	print_do_while(stmt_do_while);
	else if (auto stmt_break = rtti::cast<StmtBreak>(stmt))			print_break(stmt_break);
	else if (auto stmt_continue = rtti::cast<StmtContinue>(stmt))	print_continue(stmt_continue);
	else if (auto stmt_return = rtti::cast<StmtReturn>(stmt))		print_return(stmt_return);
	else if (auto expr = rtti::cast<Expr>(stmt))					print_expr(expr);
}

void ast::Printer::print_if(StmtIf* stmt_if)
{
	++curr_level;

	PRINT_TABS_NL(Blue, curr_level, "if");

	print_expr(stmt_if->expr);
	print_body(stmt_if->if_body);

	for (auto else_if : stmt_if->ifs)
	{
		PRINT_TABS_NL(Blue, curr_level, "else if");

		print_expr(else_if->expr);
		print_body(else_if->if_body);
	}

	if (stmt_if->else_body)
	{
		PRINT_TABS_NL(Blue, curr_level, "else");

		print_body(stmt_if->else_body);
	}

	--curr_level;
}

void ast::Printer::print_for(ast::StmtFor* stmt_for)
{
	++curr_level;

	PRINT_TABS_NL(Blue, curr_level, "for");

	print_stmt(stmt_for->init);
	print_expr(stmt_for->condition);
	print_stmt(stmt_for->step);
	print_body(stmt_for->body);

	--curr_level;
}

void ast::Printer::print_while(StmtWhile* stmt_while)
{
	++curr_level;

	PRINT_TABS_NL(Blue, curr_level, "while");

	print_expr(stmt_while->condition);
	print_body(stmt_while->body);

	--curr_level;
}

void ast::Printer::print_do_while(StmtDoWhile* stmt_do_while)
{
	++curr_level;

	PRINT_TABS_NL(Blue, curr_level, "do");

	print_body(stmt_do_while->body);

	PRINT_TABS_NL(Blue, curr_level, "while");

	print_expr(stmt_do_while->condition);

	--curr_level;
}

void ast::Printer::print_break(StmtBreak* stmt_break)
{
	++curr_level;

	PRINT_TABS_NL(Blue, curr_level, "break");

	--curr_level;
}

void ast::Printer::print_continue(StmtContinue* stmt_continue)
{
	++curr_level;

	PRINT_TABS_NL(Blue, curr_level, "continue");

	--curr_level;
}

void ast::Printer::print_return(ast::StmtReturn* stmt_return)
{
	PRINT_TABS_NL(Blue, curr_level, "return");

	print_expr(stmt_return->expr);
}

void ast::Printer::print_expr(Expr* expr)
{
	++curr_level;

	if (auto int_literal = rtti::cast<ExprIntLiteral>(expr))		print_expr_int(int_literal);
	else if (auto id = rtti::cast<ExprId>(expr))					print_id(id);
	else if (auto decl = rtti::cast<ExprDecl>(expr))				print_decl(decl);
	else if (auto assign = rtti::cast<ExprAssign>(expr))			print_assign(assign);
	else if (auto bin_assign = rtti::cast<ExprBinaryAssign>(expr))  print_binary_assign(bin_assign);
	else if (auto binary_op = rtti::cast<ExprBinaryOp>(expr))		print_expr_binary_op(binary_op);
	else if (auto unary_op = rtti::cast<ExprUnaryOp>(expr))			print_expr_unary_op(unary_op);
	else if (auto call = rtti::cast<ExprCall>(expr))				print_expr_call(call);
	else if (auto cast = rtti::cast<ExprCast>(expr))				print_cast(cast);

	--curr_level;
}

void ast::Printer::print_decl(ExprDecl* decl)
{
	PRINT_TABS_NL(Yellow, curr_level, "decl.{} ({}) '{}'", decl->rhs ? " assignment" : "", decl->type.str(), decl->name);

	if (decl->rhs)
		print_expr(decl->rhs);
}

void ast::Printer::print_assign(ExprAssign* assign)
{
	PRINT_TABS_NL(Yellow, curr_level, "assignment");

	print_expr(assign->lhs);
	print_expr(assign->rhs);
}

void ast::Printer::print_binary_assign(ExprBinaryAssign* bin_assign)
{
	PRINT_TABS_NL(Yellow, curr_level, "binary assignment ({})", STRIFY_BIN_OP(bin_assign->op));

	print_expr(bin_assign->lhs);
	print_expr(bin_assign->rhs);
}

void ast::Printer::print_expr_int(ExprIntLiteral* expr)
{
	if (expr->type.flags & TypeFlag_Unsigned)
	{
		switch (auto size = expr->type.get_size())
		{
		case 8:  PRINT_TABS_NL(Yellow, curr_level, "int (u{}) '{}'", size, expr->integer.u8);  break;
		case 16: PRINT_TABS_NL(Yellow, curr_level, "int (u{}) '{}'", size, expr->integer.u16); break;
		case 32: PRINT_TABS_NL(Yellow, curr_level, "int (u{}) '{}'", size, expr->integer.u32); break;
		case 64: PRINT_TABS_NL(Yellow, curr_level, "int (u{}) '{}'", size, expr->integer.u64); break;
		}
	}
	else
	{
		switch (auto size = expr->type.get_size())
		{
		case 8:  PRINT_TABS_NL(Yellow, curr_level, "int (i{}) '{}'", size, expr->integer.i8);  break;
		case 16: PRINT_TABS_NL(Yellow, curr_level, "int (i{}) '{}'", size, expr->integer.i16); break;
		case 32: PRINT_TABS_NL(Yellow, curr_level, "int (i{}) '{}'", size, expr->integer.i32); break;
		case 64: PRINT_TABS_NL(Yellow, curr_level, "int (i{}) '{}'", size, expr->integer.i64); break;
		}
	}
}

void ast::Printer::print_id(ast::ExprId* expr)
{
	PRINT_TABS_NL(Yellow, curr_level, "id ({}) '{}'", expr->type.str(), expr->name);
}

void ast::Printer::print_expr_unary_op(ast::ExprUnaryOp* expr)
{
	PRINT_TABS_NL(Yellow, curr_level, "unary op {} ({})", expr->lhs ? "on left side" : "on right side", STRIFY_UNARY_OP(expr->op));

	++curr_level;

	print_expr(expr->lhs ? expr->lhs : expr->rhs);

	--curr_level;
}

void ast::Printer::print_expr_binary_op(ExprBinaryOp* expr)
{
	PRINT_TABS_NL(Yellow, curr_level, "binary op ({})", STRIFY_BIN_OP(expr->op));

	++curr_level;

	if (expr->lhs) print_expr(expr->lhs);
	if (expr->rhs) print_expr(expr->rhs);

	--curr_level;
}

void ast::Printer::print_expr_call(ExprCall* expr)
{
	PRINT_TABS_NL(Yellow, curr_level, "prototype call ({})", expr->name);

	for (auto param : expr->exprs)
	{
		if (auto call = rtti::cast<ExprCall>(param))
		{
			++curr_level;

			print_expr_call(call);

			--curr_level;
		}
		else
		{
			PRINT_TABS_NL(Yellow, curr_level, "param:");
			print_expr(param);
		}
	}
}

void ast::Printer::print_cast(ExprCast* expr)
{
	PRINT_TABS_NL(Yellow, curr_level, "{} cast {} {} to {}",
		expr->implicit ? "implicit" : "explicit",
		expr->rhs->type.str(),
		expr->rhs->name,
		expr->type.str());

	print_expr(expr->rhs);
}