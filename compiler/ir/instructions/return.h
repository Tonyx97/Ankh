#pragma once

#include "instruction.h"

namespace ir
{
	struct Return : public Instruction
	{
		Type type {};

		Return()								{ item_type = ItemType_Return; }

		void print();

		static bool check_class(ItemBase* i)	{ return i->item_type == ItemType_Return; }
	};
}