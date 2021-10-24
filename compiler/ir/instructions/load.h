#pragma once

#include "instruction.h"

namespace ir
{
	struct Value;

	struct Load : public Instruction
	{
		Value* v1 = nullptr;

		Load()									{ item_type = ItemType_Load; }
		~Load()									{ _FREE(v1); }

		void print();

		static bool check_class(ItemBase* i)	{ return i->item_type == ItemType_Load; }
	};
}