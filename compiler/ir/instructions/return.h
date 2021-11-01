#pragma once

#include "instruction.h"

namespace ir
{
	struct Return : public Instruction
	{
		Value* v = nullptr;

		Return() { value_type = ValueType_Return; }

		void print();

		static bool check_class(Value* v) { return v->value_type == ValueType_Return; }
	};
}