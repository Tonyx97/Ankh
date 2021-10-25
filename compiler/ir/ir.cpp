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
	prototype->ret_type = ast_prototype->type.to_ir_type();

	for (auto param : ast_prototype->params)
		prototype->add_parameter(param->type.to_ir_type(), param->name);

	// we add the prototype here because a call inside this prototype
	// could make it recursive

	add_prototype(prototype);

	// add entry block

	prototype->add_new_block();

	// create the stack alloc and stores for parameters

	for (auto param : prototype->params)
	{
		auto stack_alloc = prototype->create_item<ir::StackAlloc>();
		auto store = prototype->create_item<ir::Store>();
		auto new_value_type = ir::Type { .type = param->type.type, .indirection = param->type.indirection + 1 };
		auto new_param_value = prototype->add_new_value_id(new_value_type, param->name);

		stack_alloc->v = param->param_value = new_param_value;
		store->v = stack_alloc->v;
		store->v1 = param;

		prototype->add_items(stack_alloc, store);
	}

	if (ast_prototype->body)
		prototype->body = generate_body(ast_prototype->body);

	if (!prototype->has_returns())
		prototype->add_return(prototype->create_item<ir::Return>());

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
	if (auto int_literal = rtti::cast<ast::ExprIntLiteral>(expr))			return generate_expr_int_literal(int_literal);
	else if (auto static_val = rtti::cast<ast::ExprStaticValue>(expr))		return generate_expr_static_value(static_val);
	else if (auto decl = rtti::cast<ast::ExprDecl>(expr))					return generate_expr_decl(decl);
	else if (auto assign = rtti::cast<ast::ExprAssign>(expr))				return generate_expr_assign(assign);
	else if (auto cast = rtti::cast<ast::ExprCast>(expr))					return generate_expr_cast(cast);
	else if (auto id = rtti::cast<ast::ExprId>(expr))						return generate_expr_id(id);
	else if (auto bin_op = rtti::cast<ast::ExprBinaryOp>(expr))				return generate_expr_binary_op(bin_op);
	else if (auto unary_op = rtti::cast<ast::ExprUnaryOp>(expr))			return generate_expr_unary_op(unary_op);
	else if (auto call = rtti::cast<ast::ExprCall>(expr))					return generate_expr_call(call);

	global_error("Could not generate the IR equivalent of an expression");

	return nullptr;
}

ir::ItemBase* IR::generate_expr_int_literal(ast::ExprIntLiteral* expr)
{
	auto v = ctx.pt->add_new_value_int(expr->type.to_ir_type(), std::to_string(expr->type.integer.u64));

	return (v->v = v);
}

ir::ItemBase* IR::generate_expr_static_value(ast::ExprStaticValue* expr)
{
	auto v = ctx.pt->add_new_value_int(expr->type.to_ir_type(), std::to_string(expr->type.integer.u64));

	return (v->v = v);
}

ir::ItemBase* IR::generate_expr_decl(ast::ExprDecl* expr)
{
	auto stack_alloc = ctx.pt->create_item<ir::StackAlloc>();
	auto value_type = expr->type.to_ir_type(1);

	stack_alloc->v = ctx.pt->add_new_value_id(value_type, expr->name);

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

ir::ItemBase* IR::generate_expr_assign(ast::ExprAssign* expr)
{
	auto store = ctx.pt->create_item<ir::Store>();

	store->v = ctx.pt->find_value(expr->name);
	store->v1 = generate_expr(expr->rhs)->v;

	ctx.pt->add_item(store);

	return store;
}

ir::ItemBase* IR::generate_expr_cast(ast::ExprCast* expr)
{
	if (expr->needs_ir_cast())
	{
		auto cast = ctx.pt->create_item<ir::Cast>();

		cast->v1 = generate_expr(expr->rhs)->v;
		cast->v = ctx.pt->add_new_value_id(expr->type.to_ir_type());

		return ctx.pt->add_item(cast);
	}

	return generate_expr(expr->rhs);
}

ir::ItemBase* IR::generate_expr_id(ast::ExprId* expr)
{
	auto existing_value = ctx.pt->find_value(expr->name);
	if (existing_value->type.indirection == 0)
		return existing_value;

	auto load = ctx.pt->create_item<ir::Load>();
	auto load_value_type = expr->type.to_ir_type();

	load->v = ctx.pt->add_new_value_id(load_value_type);
	load->v1 = existing_value->clone();

	return ctx.pt->add_item(load);
}

ir::ItemBase* IR::generate_expr_binary_op(ast::ExprBinaryOp* expr)
{
	auto bin_op = ctx.pt->create_item<ir::BinaryOp>();

	bin_op->op_type = expr->op;
	bin_op->v1 = generate_expr(expr->lhs)->v;
	bin_op->v2 = generate_expr(expr->rhs)->v;
	bin_op->v = ctx.pt->add_new_value_id(expr->type.to_ir_type());

	/*if (rtti::cast<ast::ExprId>(expr->lhs))
	{
		auto existing_value = ctx.pt->find_value(expr->name);
		if (!existing_value)
			return nullptr;

		auto store = ctx.pt->create_item<ir::Store>();

		store->v = existing_value;
		store->v1 = bin_op->v;

		ctx.pt->add_item(store);
	}*/

	return ctx.pt->add_item(bin_op);
}

ir::ItemBase* IR::generate_expr_unary_op(ast::ExprUnaryOp* expr)
{
	auto unary_op = ctx.pt->create_item<ir::UnaryOp>();
	auto store = ctx.pt->create_item<ir::Store>();

	unary_op->op_type = expr->op;
	unary_op->v1 = generate_expr(expr->rhs)->v;
	unary_op->v = ctx.pt->add_new_value_id(expr->type.to_ir_type());

	ctx.pt->add_item(unary_op);

	if (rtti::cast<ast::ExprId>(expr->rhs))
	{
		auto existing_value = ctx.pt->find_value(expr->rhs->name);
		if (!existing_value)
			return nullptr;

		store->v = existing_value;
		store->v1 = unary_op->v;

		ctx.pt->add_item(store);
	}
	
	if (expr->on_left)
		return store->v1;

	return unary_op->v1;
}

ir::ItemBase* IR::generate_expr_call(ast::ExprCall* expr)
{
	auto call = ctx.pt->create_item<ir::Call>();
	auto called_pt = ctx.find_prototype(expr->name);

	call->prototype = called_pt;
	call->type = called_pt->ret_type;
	call->v = ctx.pt->add_new_value_id(call->type);

	for (auto arg : expr->stmts)
		call->args.push_back(generate_expr(arg)->v);

	return ctx.pt->add_item(call);
}

ir::Return* IR::generate_return(ast::StmtReturn* ast_return)
{
	auto ret = ctx.pt->create_item<ir::Return>();

	if (auto expr = ast_return->expr)
	{
		ret->v = generate_expr(expr)->v;
		ret->type = ret->v->type;
	}

	return ctx.pt->add_return(ret);
}