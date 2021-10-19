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
	pi.decls.insert({ expr->get_name(), expr });
}

bool Semantic::run()
{
	ast_tree = g_syntax->get_ast();

	for (auto prototype : ast_tree->prototypes)
		analyze_prototype(prototype);

	for (const auto& call_name : gi.calls)
		if (!gi.calls.contains(call_name))
			add_error("Prototype '{}' does not exist", call_name.c_str());

	return errors.empty();
}

bool Semantic::analyze_prototype(ast::Prototype* prototype)
{
	// prototypes can be used anywhere in the global scope, there is no
	// need to declare them, we should just define them.

	pi = prototype;
	
	for (auto param_base : prototype->params)
	{
		auto param_decl = rtti::cast<ast::ExprDecl>(param_base);

		if (pi.decls.find(param_decl->id_token->value) != pi.decls.end())
			add_error("Parameter '{}' already defined in prototype declaration", param_decl->id_token->value);

		add_variable(param_decl);
	}

	if (auto body = prototype->body)
		analyze_body(body);

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
		// check if variable is defined, if so, we assign the specific variable type
		// for example, "i32 x = 0" would be Token_Id first and then Token_I32 after.

		if (auto variable = get_declared_variable(id->token->value))
			id->token->convert_to_type(variable->type_token);
		else add_error("'{}' identifier is undefined", id->token->value);
	}
	else if (auto decl = rtti::cast<ast::ExprDecl>(expr))
	{
		if (get_declared_variable(decl->id_token->value))
			add_error("'{} {}' redefinition", Lexer::STRIFY_TYPE(decl->type_token), decl->id_token->value);

		add_variable(decl);

		if (decl->rhs)
			return analyze_expr(decl->rhs);
	}
	else if (auto assign = rtti::cast<ast::ExprAssign>(expr))
	{
		if (!get_declared_variable(assign->id_token->value))
			add_error("'{}' identifier is undefined", assign->id_token->value);

		if (assign->rhs)
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
			if (!rtti::cast<ast::ExprId>(unary_op->rhs) &&
				!rtti::cast<ast::ExprIntLiteral>(unary_op->rhs))
				add_error("Expression must be an lvalue");

			return analyze_expr(unary_op->rhs);
		}
	}
	else if (auto call = rtti::cast<ast::ExprCall>(expr))
	{
		if (call->built_in)
			return true;

		const auto& prototype_name = call->id_token->value;

		auto prototype = get_prototype(prototype_name);
		if (!prototype)
			return add_error("Identifier '{}' not found", prototype_name.c_str());

		const auto original_params_length = prototype->params.size(),
				   current_params_length = call->stmts.size();

		if (current_params_length < original_params_length)
			add_error("Too few arguments in function call '{}'", prototype_name.c_str());
		else if (current_params_length > original_params_length)
			add_error("Too many arguments in function call '{}'", prototype_name.c_str());

		for (size_t i = 0ull; i < call->stmts.size() && i < prototype->params.size(); ++i)
		{
			const auto& original_param = rtti::cast<ast::ExprDecl>(prototype->params[i]);
			const auto& current_param = call->stmts[i];

			if (!analyze_expr(current_param))
				return false;

			if (!original_param->type_token->is_same_type(current_param->get_token()))
				add_error("Argument of type '{}' is incompatible with parameter of type '{}'",
						  Lexer::STRIFY_TYPE(current_param->get_token()),
						  Lexer::STRIFY_TYPE(original_param->type_token));
		}

		call->prototype = prototype;

		gi.calls.insert(prototype_name);
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

	if (stmt_return->expr ? pi.pt->ret_token->is_same_type(stmt_return->expr->get_token()) : pi.pt->ret_token->is_same_type(Token_Void))
		return true;

	add_error("Return type '{}' does not match with function type '{}'",
		Lexer::STRIFY_TYPE(stmt_return->expr ? stmt_return->expr->get_token()->id : Token_Void),
		Lexer::STRIFY_TYPE(pi.pt->ret_token));

	return false;
}

ast::ExprDecl* Semantic::get_declared_variable(const std::string& name)
{
	auto it = pi.decls.find(name);
	return (it != pi.decls.end() ? it->second : nullptr);
}

ast::Prototype* Semantic::get_prototype(const std::string& name)
{
	for (auto prototype : ast_tree->prototypes)
		if (prototype->id_token->value == name)
			return prototype;

	return nullptr;
}