#pragma once

#include "instruction.h"

namespace ir
{
	struct Value;

	struct Cast : public Instruction
	{
		Value* v1 = nullptr;

		Cast()									{ base_type = ItemType_Cast; }
		~Cast()									{ _FREE(v1); }

		void print();

		static bool check_class(ItemBase* i)	{ return i->base_type == ItemType_Cast; }
	};
}