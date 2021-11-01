#pragma once

#include "instruction.h"

namespace ir
{
	struct Value;

	struct Store : public Instruction
	{
		Value* ptr = nullptr,
			 * v = nullptr;

		Store()								{ value_type = ValueType_Store; }

		void print();

		static bool check_class(Value* v)	{ return v->value_type == ValueType_Store; }
	};
}