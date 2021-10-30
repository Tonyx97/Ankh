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
	auto v = ctx.pt->add_new_value_int(expr->type.to_ir_type(), expr->int_to_str());

	return (v->v = v);
}

ir::ItemBase* IR::generate_expr_decl(ast::ExprDecl* expr)
{
	auto stack_alloc = ctx.pt->create_item<ir::StackAlloc>();

	stack_alloc->v = ctx.pt->add_new_value_id(expr->type.to_ir_type(1), expr->name);
	stack_alloc->v->in_stack = true;	// testing

	ctx.pt->add_item(stack_alloc);

	if (expr->rhs)
	{
		auto store = ctx.pt->create_item<ir::Store>();
		auto rhs_value = generate_expr(expr->rhs)->v;

		const auto& expr_rhs_type = expr->rhs->type.to_ir_type();

		store->v = stack_alloc->v;

		if (expr_rhs_type != rhs_value->type)
		{
			auto stack_load = ctx.pt->create_item<ir::Load>();

			stack_load->v = ctx.pt->add_new_value_id(expr_rhs_type);
			stack_load->v1 = rhs_value;

			store->v1 = stack_load->v;

			ctx.pt->add_items(stack_load, store);

			return stack_load;
		}
		else
		{
			store->v1 = rhs_value;

			ctx.pt->add_item(store);
		}
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
	return ctx.pt->find_value(expr->name);
}

ir::ItemBase* IR::generate_expr_binary_op(ast::ExprBinaryOp* expr)
{
	auto bin_op = ctx.pt->create_item<ir::BinaryOp>();

	auto lhs_expr = generate_expr(expr->lhs)->v,
		 rhs_expr = generate_expr(expr->rhs)->v;

	const auto& bin_op_type = expr->lhs->type.to_ir_type();

	ir::Value* loaded_lhs = lhs_expr,
			 * loaded_rhs = rhs_expr;

	if (lhs_expr->in_stack)
	{
		auto load = ctx.pt->create_item<ir::Load>();

		load->v = ctx.pt->add_new_value_id(bin_op_type);
		load->v1 = lhs_expr;

		loaded_lhs = ctx.pt->add_item(load)->v;
	}

	if (rhs_expr->in_stack)
	{
		auto load = ctx.pt->create_item<ir::Load>();

		load->v = ctx.pt->add_new_value_id(bin_op_type);
		load->v1 = rhs_expr;

		loaded_rhs = ctx.pt->add_item(load)->v;
	}

	bin_op->op_type = expr->op;
	bin_op->v1 = loaded_lhs;
	bin_op->v2 = loaded_rhs;
	bin_op->v = ctx.pt->add_new_value_id(bin_op_type);

	return ctx.pt->add_item(bin_op);
}

ir::ItemBase* IR::generate_expr_unary_op(ast::ExprUnaryOp* expr)
{
	switch (expr->op)
	{
	case UnaryOpType_And: return generate_expr(expr->rhs);
	case UnaryOpType_Mul:
	{
		auto rhs_value = generate_expr(expr->rhs)->v;

		const auto& expr_type = expr->type.to_ir_type(),
					rhs_type = expr->rhs->type.to_ir_type();

		if (rhs_value->in_stack)
		{
			if (rhs_type == rhs_value->type)
				return rhs_value;

			auto stack_load = ctx.pt->create_item<ir::Load>();
			auto load = ctx.pt->create_item<ir::Load>();

			stack_load->v = ctx.pt->add_new_value_id(rhs_type);
			stack_load->v1 = rhs_value;

			load->v = ctx.pt->add_new_value_id(expr_type);
			load->v1 = stack_load->v;

			ctx.pt->add_item(stack_load);

			return ctx.pt->add_item(load);
		}
		else
		{
			if (expr_type == rhs_value->type)
				return rhs_value;

			auto load = ctx.pt->create_item<ir::Load>();

			load->v = ctx.pt->add_new_value_id(expr_type);
			load->v1 = rhs_value;

			return ctx.pt->add_item(load);
		}
	}
	default:
	{
		// check when we need to load.

		const auto& unary_op_type = expr->type.to_ir_type();
		auto unary_op = ctx.pt->create_item<ir::UnaryOp>();
		auto rhs_value = generate_expr(expr->rhs)->v;

		unary_op->op_type = expr->op;

		ir::Value* temp_value = nullptr;
		ir::Store* store = nullptr;

		if (rhs_value->type != unary_op_type)	// maybe this check should be rhs_value->in_stack instead
		{
			auto load = ctx.pt->create_item<ir::Load>();

			load->v = ctx.pt->add_new_value_id(unary_op_type);
			load->v1 = rhs_value;

			ctx.pt->add_item(load);

			unary_op->v = ctx.pt->add_new_value_id(unary_op_type);
			unary_op->v1 = temp_value = load->v;

			store = ctx.pt->create_item<ir::Store>();

			store->v = rhs_value;
			store->v1 = unary_op->v;
		}
		else
		{
			unary_op->v = temp_value = ctx.pt->add_new_value_id(unary_op_type);
			unary_op->v1 = rhs_value;
		}

		ctx.pt->add_item(unary_op);

		if (store)
			ctx.pt->add_item(store);

		return temp_value;
	}
	}

	return nullptr;
}

ir::ItemBase* IR::generate_expr_call(ast::ExprCall* expr)
{
	auto call = ctx.pt->create_item<ir::Call>();
	auto called_pt = ctx.find_prototype(expr->name);

	call->prototype = called_pt;
	call->type = called_pt->ret_type;
	call->v = ctx.pt->add_new_value_id(call->type);

	for (auto arg : expr->exprs)
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