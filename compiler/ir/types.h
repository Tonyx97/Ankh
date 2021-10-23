#pragma once

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

	enum BinOpType
	{
		BinOpType_None,
		BinOpType_AddAssign,
		BinOpType_Add,
		BinOpType_SubAssign,
		BinOpType_Sub,
		BinOpType_MulAssign,
		BinOpType_Mul,
		BinOpType_DivAssign,
		BinOpType_Div,
		BinOpType_ModAssign,
		BinOpType_Mod,
		BinOpType_AndAssign,
		BinOpType_And,
		BinOpType_OrAssign,
		BinOpType_Or,
		BinOpType_XorAssign,
		BinOpType_Xor,
		BinOpType_ShrAssign,
		BinOpType_Shr,
		BinOpType_ShlAssign,
		BinOpType_Shl,
		BinOpType_Equal,
		BinOpType_NotEqual,
		BinOpType_Lt,
		BinOpType_Lte,
		BinOpType_Gt,
		BinOpType_Gte,
		BinOpType_LogicalAnd,
		BinOpType_LogicalOr,
	};

	enum UnaryOpType
	{
		UnaryOpType_Add,
		UnaryOpType_Sub,
		UnaryOpType_Mul,
		UnaryOpType_And,
		UnaryOpType_Not,
		UnaryOpType_Inc,
		UnaryOpType_Dec,
		UnaryOpType_LogicalNot,
		UnaryOpType_LogicalAnd,
	};

	enum TypeID
	{
		Type_None,
		Type_Void,
		Type_i8,
		Type_i16,
		Type_i32,
		Type_i64
	};

	struct Type
	{
		TypeID type;

		int indirection;
	};
}