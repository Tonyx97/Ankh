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
	};
}