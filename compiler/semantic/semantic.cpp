#include <defs.h>

#include "semantic.h"

#include <syntax/syntax.h>

void Semantic::print_errors()
{
	for (const auto& err : errors)
		PRINT(Red, err);
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
	pi = prototype;

	add_prototype(prototype->id_token->value);

	// we have to check if the params and their types matches with the declaration
	// the problem is when a function tries to use a function that was declared as X
	// and it's used as Y
	
	for (auto param_base : prototype->params)
	{
		auto param = rtti::cast<ast::ExprDeclOrAssign>(param_base);

		if (pi.vars.find(param->id_token->value) != pi.vars.end())
			add_error("Parameter '{}' already defined in prototype declaration", param->id_token->value);

		add_variable(param);
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
		//else if (auto stmt_if = rtti::cast<ast::StmtIf>(stmt))				analyze_if(stmt_if);
		else if (auto stmt_return = rtti::cast<ast::StmtReturn>(stmt))		analyze_return(stmt_return);
	}

	return true;
}

bool Semantic::analyze_expr(ast::Expr* expr)
{
	if (auto id = rtti::cast<ast::ExprId>(expr))
	{
		if (auto variable = get_declared_variable(id->token->value))
			id->set_token(variable->type_token);
		else add_error("'{}' identifier is undefined", id->token->value);
	}
	else if (auto decl_or_assign = rtti::cast<ast::ExprDeclOrAssign>(expr))
	{
		auto declared_var = get_declared_variable(decl_or_assign->id_token->value);

		if (decl_or_assign->is_declaration())
		{
			if (declared_var)
				add_error("'{} {}' redefinition", Lexer::STRIFY_TYPE(decl_or_assign->type_token), decl_or_assign->id_token->value);

			add_variable(decl_or_assign);
		}
		else if (!declared_var)
			add_error("'{}' identifier is undefined", decl_or_assign->id_token->value);

		if (decl_or_assign->value)
			return analyze_expr(decl_or_assign->value);
	}
	else if (auto binary_op = rtti::cast<ast::ExprBinaryOp>(expr))
	{
		if (!binary_op->left || !analyze_expr(binary_op->left))
			add_error("Expected an expression");

		if (!binary_op->right || !analyze_expr(binary_op->right))
			add_error("Expected an expression");
	}
	else if (auto unary_op = rtti::cast<ast::ExprUnaryOp>(expr))
	{
		if (auto value_unary_op = rtti::cast<ast::ExprUnaryOp>(unary_op->value))
			return analyze_expr(value_unary_op);
		else
		{
			if (!rtti::cast<ast::ExprId>(unary_op->value) &&
				!rtti::cast<ast::ExprIntLiteral>(unary_op->value))
				add_error("Expression must be an lvalue");

			return analyze_expr(unary_op->value);
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
			const auto& original_param = rtti::cast<ast::ExprDeclOrAssign>(prototype->params[i]);
			const auto& current_param = call->stmts[i];

			if (!analyze_expr(current_param))
				return false;

			if (original_param->type_token->id != current_param->get_token()->id)
				add_error("Argument of type '{}' is incompatible with parameter of type '{}'",
						  Lexer::STRIFY_TYPE(current_param->get_token()),
						  Lexer::STRIFY_TYPE(original_param->type_token));
		}

		call->prototype = prototype;

		gi.calls.insert(prototype_name);
	}

	return true;
}

/*bool Semantic::analyze_if(ast::StmtIf* stmt_if)
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

	for (auto&& else_if : stmt_if->ifs)
		if (!internal_analyze_if(else_if))
			return false;

	return true;
}*/

bool Semantic::analyze_return(ast::StmtReturn* stmt_return)
{
	if (stmt_return->expr && !analyze_expr(stmt_return->expr))
		return false;

	if (stmt_return->expr ? stmt_return->expr->get_token() == pi.curr_prototype->ret_token : pi.curr_prototype->ret_token->id == Token_Void)
		return true;

	add_error("There is no valid conversion from '{}' to '{}'", Lexer::STRIFY_TYPE(stmt_return->expr ? stmt_return->expr->get_token()->id : Token_Void), Lexer::STRIFY_TYPE(pi.curr_prototype->ret_token));

	return false;
}

void Semantic::add_prototype(const std::string& name)
{
	gi.prototypes.insert(name);
}

void Semantic::add_variable(ast::ExprDeclOrAssign* expr)
{
	pi.vars.insert({ expr->get_name(), expr});
}

ast::ExprDeclOrAssign* Semantic::get_declared_variable(const std::string& name)
{
	auto it = pi.vars.find(name);
	return (it != pi.vars.end() ? it->second : nullptr);
}

ast::Prototype* Semantic::get_prototype(const std::string& name)
{
	for (auto&& prototype : ast_tree->prototypes)
		if (prototype->id_token->value == name)
			return prototype;

	return nullptr;
}