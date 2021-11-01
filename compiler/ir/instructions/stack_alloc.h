#pragma once

#include "instruction.h"

namespace ir
{
	struct StackAlloc : public Instruction
	{
		int size = 1;

		StackAlloc()						{ value_type = ValueType_StackAlloc; }

		void print();

		static bool check_class(Value* v)	{ return v->value_type == ValueType_StackAlloc; }
	};
}