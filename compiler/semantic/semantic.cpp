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
		else if (auto prototype_def = it->second; !prototype_def->type.is_same_type(prototype_decl->type))
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
			add_error("Parameter '{}' already defined in prototype declaration", param_decl->name);

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
		if (!get_declared_variable(id->name))
			add_error("'{}' identifier is undefined", id->name);
	}
	else if (auto decl = rtti::cast<ast::ExprDecl>(expr))
	{
		if (get_declared_variable(decl->name))
			add_error("'{} {}' redefinition", decl->type.str(), decl->name);

		add_variable(decl);

		auto ok = analyze_expr(decl->rhs);

		if (decl->rhs)
		{
			if (decl->type != decl->rhs->type)
				add_error("Cannot convert from type '{}' to '{}'",
					decl->rhs ? decl->rhs->type.str_full() : STRIFY_TYPE(Type_Void),
					decl->type.str_full());
		}

		return ok;
	}
	else if (auto assign = rtti::cast<ast::ExprAssign>(expr))
	{
		if (!get_declared_variable(assign->name))
			add_error("'{}' identifier is undefined", assign->name);

		auto ok = analyze_expr(assign->rhs);

		if (assign->type != assign->rhs->type)
			add_error("Cannot convert from type '{}' to '{}'",
				assign->rhs ? assign->rhs->type.str_full() : STRIFY_TYPE(Type_Void),
				assign->type.str_full());

		return ok;
	}
	else if (auto binary_op = rtti::cast<ast::ExprBinaryOp>(expr))
	{
		if (!binary_op->lhs || !analyze_expr(binary_op->lhs))
			add_error("Expected an expression");

		if (!binary_op->rhs || !analyze_expr(binary_op->rhs))
			add_error("Expected an expression");

		// update the binary op type
		// this is temporary until we add gep to ast I think
		
		binary_op->type = binary_op->lhs->type;
	}
	else if (auto unary_op = rtti::cast<ast::ExprUnaryOp>(expr))
	{
		auto ok = analyze_expr(unary_op->rhs);

		auto& unary_op_type = unary_op->type,
		    & rhs_type = unary_op->rhs->type;

		unary_op_type.update_indirection(rhs_type);

		if (unary_op->op == UnaryOpType_And || unary_op->op == UnaryOpType_Mul)
		{
			if (!rhs_type.lvalue)
				add_error("Expression must be an lvalue or function designator");

			if (unary_op->op == UnaryOpType_And)
				unary_op_type.increase_indirection();
			else if (unary_op->op == UnaryOpType_Mul)
				if (!unary_op_type.decrease_indirection())
					add_error("Cannot deref non-pointer");
		}

		return ok;
	}
	else if (auto cast = rtti::cast<ast::ExprCast>(expr))
	{
		auto ok = analyze_expr(cast->rhs);

		return ok;
	}
	else if (auto call = rtti::cast<ast::ExprCall>(expr))
	{
		if (call->built_in)
			return true;

		const auto& prototype_name = call->name;

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

			if (!original_param->type.is_same_type(current_param->type))
				add_error("Argument of type '{}' is incompatible with parameter of type '{}'",
						  current_param->type.str(),
						  original_param->type.str());
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

	if (stmt_return->expr ? p_ctx.pt->type.is_same_type(stmt_return->expr->type) : p_ctx.pt->type.is_same_type(Type_Void))
		return true;

	add_error("Return type '{}' does not match with function type '{}'",
		stmt_return->expr ? stmt_return->expr->type.str_full() : STRIFY_TYPE(Type_Void),
		p_ctx.pt->type.str_full());

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