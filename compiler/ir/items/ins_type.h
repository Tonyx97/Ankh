#pragma once

namespace ir
{
	enum Type
	{
		Ins_Unknown,
		Ins_Body,
		Ins_ValueInt,
		Ins_ValueId,
		Ins_BinOp,
		Ins_UnaryOp,
		Ins_Call,
		Ins_StackAlloc,
		Ins_Store,
		Ins_Load,
		Ins_Block,
		Ins_BranchCond,
		Ins_Branch,
		Ins_Return,
		Ins_Phi,
	};
}