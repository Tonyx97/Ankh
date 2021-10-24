#pragma once

#include "instruction.h"

namespace ir
{
	struct StackAlloc : public Instruction
	{
		StackAlloc()							{ item_type = ItemType_StackAlloc; }

		void print();

		static bool check_class(ItemBase* i)	{ return i->item_type == ItemType_StackAlloc; }
	};
}