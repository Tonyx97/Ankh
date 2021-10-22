#include <defs.h>

#include <gv/gv.h>
#include <syntax/syntax.h>

#include "ir.h"

void IR::run()
{
	ast = g_syntax->get_ast();

	// todo - generate code for global variables

	for (auto prototype : ast->prototypes)
		generate_prototype(prototype);
}

void IR::print()
{

}

void IR::add_prototype(ir::Prototype* prototype)
{
	g_ctx.prototypes.insert({ prototype->name, prototype });
}

ir::Prototype* IR::generate_prototype(ast::Prototype* ast_prototype)
{
	if (ast_prototype->is_decl())
		return generate_prototype(ast_prototype->def);

	auto ir_prototype = p_ctx.pt = _ALLOC(ir::Prototype);

	ir_prototype->name = ast_prototype->name;
	ir_prototype->ret_ty = ast_prototype->ret_ty;

	/*for (auto param : ast_prototype->params)
	{
		auto decl_or_assign = rtti::cast<ast::ExprDecl*>(param);

		ir_prototype->add_param(new ir::PrototypeParam(decl_or_assign->name, decl_or_assign->ty));
	}*/
}

ir::Body* IR::generate_from_body(ast::StmtBody* ast_body)
{
	auto body = _ALLOC(ir::Body);

	for (auto stmt : ast_body->stmts)
	{
		if (auto body = rtti::cast<ast::StmtBody>(stmt))					generate_from_body(body);
	}

	return body;
}