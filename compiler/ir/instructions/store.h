#pragma once

#include "instruction.h"

namespace ir
{
	struct Value;

	struct Store : public Instruction
	{
		Value* v1 = nullptr;

		Store()									{ base_type = ItemType_Store; }
		~Store()								{ _FREE(v1); }

		void print();

		static bool check_class(ItemBase* i)	{ return i->base_type == ItemType_Store; }
	};
}