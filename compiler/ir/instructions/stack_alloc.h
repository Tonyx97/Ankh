#pragma once

#include "instruction.h"

namespace ir
{
	struct StackAlloc : public Instruction
	{
		StackAlloc()							{ base_type = ItemType_StackAlloc; }

		void print();

		static bool check_class(ItemBase* i)	{ return i->base_type == ItemType_StackAlloc; }
	};
}