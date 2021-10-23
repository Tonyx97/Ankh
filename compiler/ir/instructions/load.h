#pragma once

#include "instruction.h"

namespace ir
{
	struct Value;

	struct Load : public Instruction
	{
		Value* v1 = nullptr;

		Load()									{ base_type = ItemType_Load; }
		~Load()									{ _FREE(v1); }

		void print();

		static bool check_class(ItemBase* i)	{ return i->base_type == ItemType_Load; }
	};
}