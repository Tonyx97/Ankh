#pragma once

#include "instruction.h"

namespace ir
{
	struct Value;

	struct Cast : public Instruction
	{
		Value* source = nullptr;

		Cast() { value_type = ValueType_Cast; }

		void print();

		static bool check_class(Value* v) { return v->value_type == ValueType_Cast; }
	};
}