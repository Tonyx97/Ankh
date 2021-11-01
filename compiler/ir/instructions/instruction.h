#pragma once

#include <ir/items/value.h>

namespace ir
{
	struct Instruction : public Value
	{
		virtual void print() = 0;

		static bool check_class(Value* v) { return v->value_type > ValueType_BeginInstructionType && v->value_type < ValueType_EndInstructionType; }
	};
}