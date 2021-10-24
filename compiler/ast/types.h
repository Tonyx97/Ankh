#pragma once

#include <enums/ast_ir_types.h>

namespace ast
{
	enum StmtExprType
	{
		Stmt_None,
		StmtExpr,
		StmtExpr_Begin,
			StmtExpr_IntLiteral,
			StmtExpr_StaticValue,
			StmtExpr_Id,
			StmtExpr_Decl,
			StmtExpr_Assign,
			StmtExpr_BinOp,
			StmtExpr_UnaryOp,
			StmtExpr_Call,
			StmtExpr_Cast,
		StmtExpr_End,
		Stmt_Body,
		Stmt_If,
		Stmt_For,
		Stmt_While,
		Stmt_DoWhile,
		Stmt_Break,
		Stmt_Continue,
		Stmt_Return,
	};

	struct Type
	{
		Int integer;

		TypeID type = Type_None;

		uint64_t flags = TypeFlag_None;

		int indirection = -1;

		uint8_t size = 0;

		Type() {}

		bool is_same_type(const Type& v) const	{ return type == v.type; }
		bool is_same_type(TypeID v) const		{ return type == v; }

		ir::Type to_ir_type(int indirection = 0)
		{
			return { type, indirection };
		}
	};
}