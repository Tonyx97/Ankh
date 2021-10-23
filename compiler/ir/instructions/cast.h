#pragma once

#include "instruction.h"

namespace ir
{
	struct Value;

	struct Cast : public Instruction
	{
		Value* v1 = nullptr;

		Cast()									{ base_type = ItemType_Cast; }

		void print();

		static bool check_class(ItemBase* i)	{ return i->base_type == ItemType_Cast; }
	};
}