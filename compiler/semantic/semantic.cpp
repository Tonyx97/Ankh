#include <defs.h>

#include "semantic.h"

#include <syntax/syntax.h>

void Semantic::print_errors()
{
	for (const auto& err : errors)
		PRINT(Red, err);
}

void Semantic::add_variable(ast::ExprDecl* expr)
{
	p_ctx.decls.insert({ expr->name, expr });
}

bool Semantic::run()
{
	ast = g_syntax->get_ast();

	for (auto global_decl : ast->global_decls)
		analyze_expr(global_decl);

	for (auto prototype : ast->prototypes)
		analyze_prototype(prototype);

	for (const auto& [name, prototype_decl] : g_ctx.prototype_decls)
		if (auto it = g_ctx.prototype_defs.find(name); it == g_ctx.prototype_defs.end())
		{
			if (g_ctx.calls.find(name) == g_ctx.calls.end())
				add_error("Unresolved prototype '{}' declared", name);
			else add_error("Unresolved prototype '{}' referenced", name);
		}
		else if (auto prototype_def = it->second; !prototype_def->ret_type->is_same_type(prototype_decl->ret_type))
			add_error("Cannot overload prototype '{}' by return type alone", name);

	return errors.empty();
}

bool Semantic::analyze_prototype(ast::Prototype* prototype)
{
	p_ctx = prototype;

	// todo - allow prototype overloads

	for (auto param_base : prototype->params)
	{
		auto param_decl = rtti::cast<ast::ExprDecl>(param_base);

		if (p_ctx.has_decl(param_decl))
			add_error("Parameter '{}' already defined in prototype declaration", param_decl->id->value);

		add_variable(param_decl);
	}

	if (prototype->is_decl())
	{
		if (!prototype->def)
			add_error("Unresolved prototype '{}' declared", prototype->name);

		g_ctx.add_prototype_decl(prototype);
	}
	else
	{
		g_ctx.add_prototype_def(prototype);

		analyze_body(prototype->body);
	}

	return true;
}

bool Semantic::analyze_body(ast::StmtBody* body)
{
	for (auto stmt : body->stmts)
	{
		if (auto body = rtti::cast<ast::StmtBody>(stmt))					analyze_body(body);
		else if (auto expr = rtti::cast<ast::Expr>(stmt))					analyze_expr(expr);
		else if (auto stmt_if = rtti::cast<ast::StmtIf>(stmt))				analyze_if(stmt_if);
		else if (auto stmt_return = rtti::cast<ast::StmtReturn>(stmt))		analyze_return(stmt_return);
	}

	return true;
}

bool Semantic::analyze_expr(ast::Expr* expr)
{
	if (auto id = rtti::cast<ast::ExprId>(expr))
	{
		if (!get_declared_variable(id->id->value))
			add_error("'{}' identifier is undefined", id->id->value);
	}
	else if (auto decl = rtti::cast<ast::ExprDecl>(expr))
	{
		if (get_declared_variable(decl->id->value))
			add_error("'{} {}' redefinition", Lexer::STRIFY_TYPE(decl->id), decl->id->value);

		add_variable(decl);

		if (decl->rhs)
			return analyze_expr(decl->rhs);
	}
	else if (auto assign = rtti::cast<ast::ExprAssign>(expr))
	{
		if (!get_declared_variable(assign->id->value))
			add_error("'{}' identifier is undefined", assign->id->value);

		return analyze_expr(assign->rhs);
	}
	else if (auto binary_op = rtti::cast<ast::ExprBinaryOp>(expr))
	{
		if (!binary_op->lhs || !analyze_expr(binary_op->lhs))
			add_error("Expected an expression");

		if (!binary_op->rhs || !analyze_expr(binary_op->rhs))
			add_error("Expected an expression");
	}
	else if (auto unary_op = rtti::cast<ast::ExprUnaryOp>(expr))
	{
		if (auto value_unary_op = rtti::cast<ast::ExprUnaryOp>(unary_op->rhs))
			return analyze_expr(value_unary_op);
		else
		{
			// missing checks to see if rhs is actually a rvalue
			// and not lvalue

			return analyze_expr(unary_op->rhs);
		}
	}
	else if (auto call = rtti::cast<ast::ExprCall>(expr))
	{
		if (call->built_in)
			return true;

		const auto& prototype_name = call->id->value;

		auto prototype = get_declared_prototype(prototype_name);
		if (!prototype)
			return add_error("Prototype identifier '{}' not found", prototype_name);

		const auto original_params_length = prototype->params.size(),
				   current_params_length = call->stmts.size();

		if (current_params_length < original_params_length)
			add_error("Too few arguments in function call '{}'", prototype_name);
		else if (current_params_length > original_params_length)
			add_error("Too many arguments in function call '{}'", prototype_name);

		for (size_t i = 0ull; i < call->stmts.size() && i < prototype->params.size(); ++i)
		{
			const auto& original_param = rtti::cast<ast::ExprDecl>(prototype->params[i]);
			const auto& current_param = call->stmts[i];

			if (!analyze_expr(current_param))
				return false;

			if (!original_param->type->is_same_type(current_param->type))
				add_error("Argument of type '{}' is incompatible with parameter of type '{}'",
						  Lexer::STRIFY_TYPE(current_param->type),
						  Lexer::STRIFY_TYPE(original_param->type));
		}

		call->prototype = prototype;

		g_ctx.add_call(prototype_name);
	}

	return true;
}

bool Semantic::analyze_if(ast::StmtIf* stmt_if)
{
	auto internal_analyze_if = [&](ast::StmtIf* curr_if)
	{
		if (!analyze_expr(curr_if->expr))
			return false;

		if (curr_if->if_body)
			if (!analyze_body(curr_if->if_body))
				return false;

		if (curr_if->else_body)
			if (!analyze_body(curr_if->else_body))
				return false;

		return true;
	};

	if (!internal_analyze_if(stmt_if))
		return false;

	for (auto else_if : stmt_if->ifs)
		if (!internal_analyze_if(else_if))
			return false;

	return true;
}

bool Semantic::analyze_return(ast::StmtReturn* stmt_return)
{
	if (stmt_return->expr && !analyze_expr(stmt_return->expr))
		return false;

	if (stmt_return->expr ? p_ctx.pt->ret_type->is_same_type(stmt_return->expr->type) : p_ctx.pt->ret_type->is_same_type(Token_Void))
		return true;

	add_error("Return type '{}' does not match with function type '{}'",
		Lexer::STRIFY_TYPE(stmt_return->expr ? stmt_return->expr->type->id : Token_Void),
		Lexer::STRIFY_TYPE(p_ctx.pt->ret_type));

	return false;
}

ast::ExprDecl* Semantic::get_declared_variable(const std::string& name)
{
	auto it = p_ctx.decls.find(name);
	return (it != p_ctx.decls.end() ? it->second : nullptr);
}

ast::Prototype* Semantic::get_declared_prototype(const std::string& name)
{
	auto it = g_ctx.prototype_decls.find(name);
	if (it != g_ctx.prototype_decls.end())
		return it->second;

	return get_defined_prototype(name);
}

ast::Prototype* Semantic::get_defined_prototype(const std::string& name)
{
	auto it = g_ctx.prototype_defs.find(name);
	return (it != g_ctx.prototype_defs.end() ? it->second : nullptr);
}