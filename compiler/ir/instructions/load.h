#pragma once

#include "instruction.h"

namespace ir
{
	struct Value;

	struct Load : public Instruction
	{
		Value* ptr = nullptr;

		Load() { value_type = ValueType_Load; }

		void print();

		static bool check_class(Value* v) { return v->value_type == ValueType_Load; }
	};
}