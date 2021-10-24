#pragma once

#include "instruction.h"

namespace ir
{
	struct Prototype;

	struct Call : public Instruction
	{
		Type type {};

		Prototype* prototype = nullptr;

		std::vector<Value*> args;

		Call()									{ item_type = ItemType_Call; }
		~Call()
		{
			for (auto arg : args)
				_FREE(arg);
		}

		void print();

		static bool check_class(ItemBase* i)	{ return i->item_type == ItemType_Call; }
	};
}