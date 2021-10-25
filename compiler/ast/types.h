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
		Int integer = { 0 };

		TypeID type = Type_None;

		uint64_t flags = TypeFlag_None;

		int indirection = -1;

		uint8_t size = 0;

		Type() {}

		bool is_same_type(const Type& v) const			{ return type == v.type; }
		bool is_same_type(TypeID v) const				{ return type == v; }

		const Type* normal_implicit_cast(const Type& rhs) const
		{
			return (type == rhs.type ? nullptr : this);
		}

		const Type* binary_implicit_cast(const Type& rhs)
		{
			auto& lhs = *this;

			auto lhs_type = type;
			auto rhs_type = rhs.type;

			if (lhs_type == rhs_type)
				return nullptr;

			const bool lhs_signed = !(lhs.flags & TypeFlag_Unsigned),
					   rhs_signed = !(rhs.flags & TypeFlag_Unsigned);

			if (lhs_signed == rhs_signed)
			{
				if (lhs.size > rhs.size)	  return &lhs;
				else if (lhs.size < rhs.size) return &rhs;

				global_error("Invalid sides sizes while casting");
			}
			else
			{
				const Type* signed_t = nullptr,
						  * unsigned_t = nullptr;

				if (lhs_signed)
				{
					signed_t = &lhs;
					unsigned_t = &rhs;
				}
				else
				{
					signed_t = &rhs;
					unsigned_t = &lhs;
				}

				if (unsigned_t->size >= signed_t->size) return unsigned_t;
				else									return signed_t;
			}

			return nullptr;
		}

		ir::Type to_ir_type(int indirection = 0) const
		{
			return
			{
				.type = type,
				.indirection = this->indirection + indirection
			};
		}

		std::string str() const							{ return STRIFY_TYPE(type); }

		bool operator == (const Type& v) const			{ return type == v.type; }
	};
}