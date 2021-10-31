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
			StmtExpr_Id,
			StmtExpr_Decl,
			StmtExpr_Assign,
			StmtExpr_BinAssign,
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
		TypeID type = Type_None;

		uint64_t flags = TypeFlag_None;

		int indirection = -1;

		void update_indirection(const Type& v)			{ indirection = v.indirection; }
		void update_indirection(int v)					{ indirection = v; }
		void increase_indirection()						{ ++indirection; }
		bool decrease_indirection()						{ return --indirection >= 0; }
		bool is_unsigned() const						{ return (flags & TypeFlag_Unsigned); }

		bool is_same_type(const Type& v) const			{ return type == v.type && indirection == v.indirection; }
		bool is_same_type(TypeID v) const				{ return type == v && indirection == 0; }

		size_t get_size() const
		{
			if (indirection > 0)
				return sizeof(uint64_t) * 8ull;

			switch (type)
			{
			case Type_Void: return 0ull;
			case Type_u8:
			case Type_i8:	return sizeof(int8_t) * 8ull;
			case Type_u16:
			case Type_i16:	return sizeof(int16_t) * 8ull;
			case Type_u32:
			case Type_i32:	return sizeof(int32_t) * 8ull;
			case Type_u64:
			case Type_i64:	return sizeof(int64_t) * 8ull;
			}

			global_error("Invalid type while getting size");

			return 0;
		}

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
				return &lhs;

			const bool lhs_signed = !(lhs.flags & TypeFlag_Unsigned),
					   rhs_signed = !(rhs.flags & TypeFlag_Unsigned);

			if (lhs_signed == rhs_signed)
			{
				const auto lhs_size = lhs.get_size(),
						   rhs_size = rhs.get_size();

				if (lhs_size > rhs_size)	  return &lhs;
				else if (lhs_size < rhs_size) return &rhs;

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

				if (unsigned_t->get_size() >= signed_t->get_size()) return unsigned_t;
				else												return signed_t;
			}

			return &lhs;
		}

		ir::Type to_ir_type(int indirection = 0) const
		{
			TypeID new_type;

			switch (type)
			{
			case Token_Void:	new_type = Type_Void; break;
			case Token_U8:
			case Token_I8:		new_type = Type_i8;   break;
			case Token_U16:
			case Token_I16:		new_type = Type_i16;  break;
			case Token_U32:
			case Token_I32:		new_type = Type_i32;  break;
			case Token_U64:
			case Token_I64:		new_type = Type_i64;  break;
			}

			return
			{
				.type = new_type,
				.indirection = this->indirection + indirection,
			};
		}

		std::string indirection_str() const
		{
			if (indirection <= 0)
				return {};

			std::string str;

			str.resize(indirection);

			std::fill_n(str.begin(), indirection, '*');

			return str;
		}

		std::string str() const							{ return STRIFY_TYPE(type); }
		std::string str_full() const					{ return str() + indirection_str(); }

		bool operator == (const Type& v) const			{ return is_same_type(v); }
		bool operator == (TypeID v) const				{ return is_same_type(v); }
	};

	using TypeOpt = std::optional<Type>;
}