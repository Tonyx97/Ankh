#pragma once

#include <enums/ast_ir_types.h>

namespace ir
{
	enum ItemType
	{
		ItemType_None,
		ItemType_BeginValueType,
		ItemType_ValueId,
		ItemType_ValueInt,
		ItemType_EndValueType,
		ItemType_Body,
		ItemType_Cast,
		ItemType_BinOp,
		ItemType_UnaryOp,
		ItemType_Call,
		ItemType_StackAlloc,
		ItemType_Store,
		ItemType_Load,
		ItemType_Block,
		ItemType_BranchCond,
		ItemType_Branch,
		ItemType_Return,
		ItemType_Phi,
	};

	struct Type
	{
		TypeID type;

		int indirection;

		std::string str() const					{ return STRIFY_TYPE(type); }

		bool is_same_type(const Type& v) const	{ return type == v.type && indirection == v.indirection; }
		bool is_same_type(TypeID v) const		{ return type == v && indirection == 0; }
		bool operator == (const Type& v) const	{ return is_same_type(v); }
	};
}