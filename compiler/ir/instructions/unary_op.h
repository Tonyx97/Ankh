#pragma once

#include "instruction.h"

namespace ir
{
	struct UnaryOp : public Instruction
	{
		UnaryOpType op_type = UnaryOpType_None;

		Value* v1 = nullptr;

		UnaryOp()								{ base_type = ItemType_UnaryOp; }
		~UnaryOp()
		{
			_FREE(v1);
		}

		void print();

		static bool check_class(ItemBase* i)	{ return i->base_type == ItemType_UnaryOp; }
	};
}