#pragma once

#include "instruction.h"

namespace ir
{
	struct Branch : public Instruction
	{
		struct Block* target = nullptr;

		Branch()							{ value_type = ValueType_Branch; }

		void print();

		static bool check_class(Value* v)	{ return v->value_type == ValueType_Branch; }
	};
}