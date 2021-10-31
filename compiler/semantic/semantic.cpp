#include <defs.h>

#include "semantic.h"

#include <syntax/syntax.h>

void Semantic::print_errors()
{
	for (const auto& err : errors)
		PRINT(Red, err);
}

void Semantic::enter_scope()
{
}

void Semantic::exit_scope()
{
}

void Semantic::analyze_function(ast::Prototype* function)
{
	p_ctx.clear();
	p_ctx.pt = function;

	for (const auto parameter : function->params) 
		p_ctx.set_id_type(parameter->name, parameter->type);

	analyze_body(function->body, semantic::BodyData{});
}

void Semantic::analyze_body(ast::StmtBody* body, semantic::BodyData body_data)
{
	enter_scope();

	for (auto stmt : body->stmts)
	{
		if (auto body = rtti::cast<ast::StmtBody>(stmt))
			analyze_body(body, semantic::BodyData{body_data.depth + 1, body_data.is_in_loop});
		else if (auto expr = rtti::cast<ast::Expr>(stmt))					analyze_expr(expr);
		else if (auto stmt_if = rtti::cast<ast::StmtIf>(stmt))				analyze_if(stmt_if, body_data);
		else if (auto stmt_for = rtti::cast<ast::StmtFor>(stmt))			analyze_for(stmt_for, body_data);
		else if (auto stmt_while = rtti::cast<ast::StmtWhile>(stmt))		analyze_while(stmt_while, body_data);
		else if (auto stmt_do_while = rtti::cast<ast::StmtDoWhile>(stmt))	analyze_do_while(stmt_do_while, body_data);
		else if (auto stmt_return = rtti::cast<ast::StmtReturn>(stmt))		analyze_return(stmt_return, body_data);
		else if (rtti::cast<ast::StmtContinue>(stmt) || rtti::cast<ast::StmtBreak>(stmt))
		{
			if (!body_data.is_in_loop)
				add_error("Cannot use continue and break when not in loop.");
		}
	}

	exit_scope();
}

void Semantic::analyze_if(ast::StmtIf* stmt, semantic::BodyData data)
{
	enter_scope();

	analyze_expr(stmt->expr);

	for (auto if_ : stmt->ifs)
		analyze_if(if_, data);

	const auto new_data = semantic::BodyData{ data.depth + 1, data.is_in_loop };

	analyze_body(stmt->if_body, new_data);
	analyze_body(stmt->else_body, new_data);

	exit_scope();
}

void Semantic::analyze_for(ast::StmtFor* stmt, semantic::BodyData data)
{
	enter_scope();

	check(false, "TODO");

	exit_scope();
}

void Semantic::analyze_while(ast::StmtWhile* stmt, semantic::BodyData data)
{
	enter_scope();

	analyze_expr(stmt->condition);
	analyze_body(stmt->body, semantic::BodyData{ data.depth + 1, true });

	exit_scope();
}

void Semantic::analyze_do_while(ast::StmtDoWhile* stmt, semantic::BodyData data)
{
	enter_scope();

	check(false, "TODO");

	exit_scope();
}

void Semantic::analyze_return(ast::StmtReturn* stmt, semantic::BodyData data)
{
	auto return_type = p_ctx.pt->type;

	if (!stmt->expr)
	{
		if (return_type.is_same_type(TypeID::Type_Void))
			add_error("Cannot return void from non void function.");
	}
	else
	{
		implicit_cast_replace(stmt->expr, return_type);
	}
}

void Semantic::analyze_expr(ast::Expr* expr)
{
	if (!expr || p_ctx.analyzed_exprs.find(expr) != p_ctx.analyzed_exprs.end())
		return;

	p_ctx.analyzed_exprs.insert(expr);

	if (auto c_expr = rtti::cast<ast::ExprId>(expr))				analyze_expr_id(c_expr);
	else if (auto c_expr = rtti::cast<ast::ExprDecl>(expr))			analyze_expr_decl(c_expr);
	else if (auto c_expr = rtti::cast<ast::ExprAssign>(expr))		analyze_expr_assign(c_expr);
	else if (auto c_expr = rtti::cast<ast::ExprBinaryAssign>(expr)) analyze_expr_bin_assign(c_expr);
	else if (auto c_expr = rtti::cast<ast::ExprBinaryOp>(expr))		analyze_expr_bin_op(c_expr);
	else if (auto c_expr = rtti::cast<ast::ExprUnaryOp>(expr))		analyze_expr_unary_op(c_expr);
	else if (auto c_expr = rtti::cast<ast::ExprCall>(expr))			analyze_expr_call(c_expr);
	else if (auto c_expr = rtti::cast<ast::ExprCast>(expr))			analyze_expr_cast(c_expr);

	check(expr->type.type != Type_None, "???");
}

void Semantic::analyze_expr_id(ast::ExprId* expr) 
{
	expr->type = get_id_type(expr->name);
}

void Semantic::analyze_expr_decl(ast::ExprDecl* expr) 
{
	// Already has type filled.

	p_ctx.set_id_type(expr->name, expr->type);
	implicit_cast_replace(expr->rhs, expr->type);
}

void Semantic::analyze_expr_assign(ast::ExprAssign* expr) 
{
	const auto lhs_type = get_expr_type(expr->lhs);
	implicit_cast_replace(expr->rhs, lhs_type);

	expr->type = lhs_type;
}

void Semantic::analyze_expr_bin_assign(ast::ExprBinaryAssign* expr) 
{
	auto lhs_type = get_expr_type(expr->lhs);
	implicit_cast_replace(expr->rhs, lhs_type);

	expr->type = lhs_type;
}

void Semantic::analyze_expr_bin_op(ast::ExprBinaryOp* expr) {
	// TODO: 
	// 1. Check if types are arithmetic
	// 2. Handle +/- for pointers

	auto lhs_type = get_expr_type(expr->lhs);
	auto rhs_type = get_expr_type(expr->rhs);
	auto common_type = *lhs_type.binary_implicit_cast(rhs_type);

	implicit_cast_replace(expr->lhs, common_type);
	implicit_cast_replace(expr->rhs, common_type);

	expr->type = common_type;
}

void Semantic::analyze_expr_unary_op(ast::ExprUnaryOp* expr) 
{
	auto type = get_expr_type(expr->lhs ? expr->lhs : expr->rhs);

	if (expr->op == UnaryOpType_Mul) 
	{
		check(type.indirection > 0, "Cannot deref non-pointer.");
		type.decrease_indirection();
	}
	else if (expr->op == UnaryOpType_And)
	{
		type.increase_indirection();
	}

	expr->type = type;
}

void Semantic::analyze_expr_call(ast::ExprCall* expr) 
{
	for (auto param : expr->exprs)
		analyze_expr(param);

	const auto prototype = g_ctx.get_prototype(expr->name);
	if (!prototype)
	{
		add_error("Cannot call unknown function {}.", expr->name);
		return;
	}

	check(expr->exprs.size() == prototype->params.size(), "Invalid parameter count.");

	for (int i = 0; i < expr->exprs.size(); ++i)
		implicit_cast_replace(expr->exprs[i], get_expr_type(prototype->params[i]));

	expr->prototype = prototype;
	expr->type = prototype->type;
}

void Semantic::analyze_expr_cast(ast::ExprCast* expr)
{
	analyze_expr(expr->rhs);
}

ast::Type Semantic::get_expr_type(ast::Expr* expr)
{
	analyze_expr(expr);

	check(expr->type.type != Type_None, "???");

	return expr->type;
}

ast::Type Semantic::get_id_type(const std::string& id)
{
	const auto type = p_ctx.get_id_type(id);

	if (type)
	{
		return *type;
	}
	else
	{
		add_error("Undefined identifier {} used.", id);
		return ast::Type(Type_None, 0);
	}
}

ast::Expr* Semantic::implicit_cast(ast::Expr* expr, ast::Type type)
{
	const auto source_type = get_expr_type(expr);

	if (source_type == type)
		return expr;

	if (source_type.indirection > 0 || type.indirection > 0)
	{
		add_error("Cannot implicit cast pointers.");
		return expr;
	}

	return _ALLOC(ast::ExprCast, expr, type, true);
}

void Semantic::implicit_cast_replace(ast::Expr*& expr, ast::Type type)
{
	expr = implicit_cast(expr, type);
}

bool Semantic::run()
{
	ast = g_syntax->get_ast();

	for (auto prototype : ast->prototypes)
		g_ctx.add_prototype(prototype);

	for (auto prototype : ast->prototypes)
	{
		if (prototype->body)
			analyze_function(prototype);
		else
			add_error("Function {} doesn't have a body.", prototype->name);
	}

	return errors.empty();
}

void semantic::PrototypeInfo::set_id_type(const std::string& id, ast::Type type)
{
	auto it = id_types.find(id);
	if (it == id_types.end())
	{
		id_types[id] = type;
		return;
	}

	if (it->second != type)
	{
		g_semantic->add_error("ID {} has differing types.", id);
	}
}

ast::TypeOpt semantic::PrototypeInfo::get_id_type(const std::string& id)
{
	auto it = id_types.find(id);
	if (it == id_types.end())
		return std::nullopt;

	return it->second;
}