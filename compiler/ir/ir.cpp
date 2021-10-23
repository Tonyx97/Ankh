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
		else if (auto expr = rtti::cast<ast::Expr>(stmt))					generate_expr(expr);
		else if (auto stmt_return = rtti::cast<ast::StmtReturn>(stmt))		generate_return(stmt_return);
	}

	return body;
}

ir::ItemBase* IR::generate_expr(ast::Expr* expr)
{
	if (auto int_literal = rtti::cast<ast::ExprIntLiteral>(expr))			 return generate_expr_int_literal(int_literal);
	else if (auto cast = rtti::cast<ast::ExprCast>(expr))					return generate_expr_cast(cast);
	else if (auto decl = rtti::cast<ast::ExprDecl>(expr))					return generate_expr_decl(decl);
	//else if (auto id = rtti::cast<ast::ExprId>(expr))						 return generate_from_expr_id(id);
	//else if (auto binary_op = rtti::cast::ExprBinaryOp>(expr))				 return generate_from_expr_binary_op(binary_op);
	//else if (auto unary_op = rtti::cast<ast::ExprUnaryOp>(expr))			 return generate_from_expr_unary_op(unary_op);
	//else if (auto call = rtti::cast<ast::ExprCall>(expr))					 return generate_from_expr_call(call);

	global_error("Could not generate the IR equivalent of an expression");

	return nullptr;
}

ir::ItemBase* IR::generate_expr_decl(ast::ExprDecl* expr)
{
	auto stack_alloc = ctx.pt->create_item<ir::StackAlloc>();

	stack_alloc->v = ctx.pt->add_new_value_id();
	stack_alloc->v->type = expr->type->create_type();

	ctx.pt->add_item(stack_alloc);

	if (expr->rhs)
	{
		auto store = ctx.pt->create_item<ir::Store>();

		store->v = stack_alloc->v;
		store->v1 = generate_expr(expr->rhs)->v;

		ctx.pt->add_item(store);
	}

	return stack_alloc;
}

ir::ItemBase* IR::generate_expr_cast(ast::ExprCast* expr)
{
	auto cast = ctx.pt->create_item<ir::Cast>();

	cast->v1 = generate_expr(expr->rhs)->v;
	cast->v = ctx.pt->add_new_value_id();
	cast->v->type = expr->type->create_type();

	ctx.pt->add_item(cast);

	return cast;
}

ir::ItemBase* IR::generate_expr_int_literal(ast::ExprIntLiteral* expr)
{
	auto v = ctx.pt->add_new_value_int();
	auto integer = expr->id->integer.u64;

	v->vi = integer;
	v->name = std::to_string(integer);
	v->type = expr->id->create_type();

	return (v->v = v);
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

	return ctx.pt->add_item(ret);
}