#include <defs.h>

#include <gv/gv.h>
#include <syntax/syntax.h>

#include "ir.h"

IR::~IR()
{
	for (const auto& [name, pt] : ctx.prototypes)
		_FREE(pt);
}

void IR::run()
{
	ast = g_syntax->get_ast();

	// todo - generate code for global variables

	for (auto prototype : ast->prototypes)
		generate_prototype(prototype);
}

void IR::print()
{
	for (const auto& [name, pt] : ctx.prototypes)
		pt->print();
}

void IR::add_prototype(ir::Prototype* prototype)
{
	ctx.prototypes.insert({ prototype->name, prototype });
}

ir::Prototype* IR::generate_prototype(ast::Prototype* ast_prototype)
{
	if (ast_prototype->is_decl())
		return generate_prototype(ast_prototype->def);

	auto prototype = ctx.pt = _ALLOC(ir::Prototype);

	prototype->name = ast_prototype->name;
	prototype->type = ast_prototype->ret_token->create_type();

	for (auto param : ast_prototype->params)
		prototype->add_param(param->type->create_type());

	if (ast_prototype->body)
	{
		auto entry_block = prototype->add_new_block();

		prototype->body = generate_body(ast_prototype->body);

		if (ast_prototype->ret_token->is_same_type(Token_Void))
			prototype->add_new_item<ir::Return>();
	}

	add_prototype(prototype);

	return prototype;
}

ir::Body* IR::generate_body(ast::StmtBody* ast_body)
{
	auto body = _ALLOC(ir::Body);
	
	for (auto stmt : ast_body->stmts)
	{
		if (auto body = rtti::cast<ast::StmtBody>(stmt))					generate_body(body);
		else if (auto stmt_return = rtti::cast<ast::StmtReturn>(stmt))		generate_return(stmt_return);
	}

	return body;
}

ir::Return* IR::generate_return(ast::StmtReturn* ast_return)
{
	auto ret = ctx.pt->create_item<ir::Return>();

	if (auto expr = ast_return->expr)
	{
		ret->type = expr->type->create_type();

		/*if (auto op_i = ret->op_i = generate_from_expr(expr))
			op_i->get_value()->ret = ret;*/
	}

	ctx.pt->add_item(ret);

	return ret;
}