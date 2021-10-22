#include <defs.h>

#include <gv/gv.h>
#include <syntax/syntax.h>

#include "ir.h"

void IR::run()
{
	ast = g_syntax->get_ast();

	for (auto prototype : ast->prototypes)
		generate_prototype(prototype);
}

void IR::print()
{

}

ir::Prototype* IR::generate_prototype(ast::Prototype* prototype)
{
	return nullptr;
}

ir::Body* IR::generate_from_body(ast::StmtBody* body)
{
	auto ir_body = _ALLOC(ir::Body);

	for (auto stmt : body->stmts)
	{
		if (auto body = rtti::cast<ast::StmtBody>(stmt))					generate_from_body(body);
	}

	return ir_body;
}