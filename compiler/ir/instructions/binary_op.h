#pragma once

#include "instruction.h"

namespace ir
{
	struct BinaryOp : public Instruction
	{
		BinOpType op_type = BinOpType_None;

		Value* v1 = nullptr,
			 * v2 = nullptr;

		BinaryOp()								{ base_type = ItemType_BinOp; }
		~BinaryOp()
		{
			_FREE(v1);
			_FREE(v2);
		}

		void print();

		static bool check_class(ItemBase* i)	{ return i->base_type == ItemType_BinOp; }
	};
}