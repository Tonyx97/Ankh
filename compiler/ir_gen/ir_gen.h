#pragma once
#include <ir/ir.h>

/*
class IR
{
private:

	ir::Context ctx{};

	ast::AST* ast = nullptr;

public:

	~IR();

	void run();
	void print();
	void add_prototype(ir::Prototype* prototype);

	// statements

	ir::Prototype* generate_prototype(ast::Prototype* ast_prototype);
	ir::Body* generate_body(ast::StmtBody* ast_body);

	// expressions

	ir::ItemBase* generate_expr(ast::Expr* expr);
	ir::ItemBase* generate_expr_int_literal(ast::ExprIntLiteral* expr);
	ir::ItemBase* generate_expr_decl(ast::ExprDecl* expr);
	ir::ItemBase* generate_expr_assign(ast::ExprAssign* expr);
	ir::ItemBase* generate_expr_cast(ast::ExprCast* expr);
	ir::ItemBase* generate_expr_id(ast::ExprId* expr);
	ir::ItemBase* generate_expr_binary_op(ast::ExprBinaryOp* expr);
	ir::ItemBase* generate_expr_unary_op(ast::ExprUnaryOp* expr);
	ir::ItemBase* generate_expr_call(ast::ExprCall* expr);

	ir::Return* generate_return(ast::StmtReturn* ast_return);
};

inline std::unique_ptr<IR> g_ir;
*/