#pragma once

#include "instruction.h"

namespace ir
{
	struct Value;
	struct Block;

	struct BranchCond : public Instruction
	{
		Value* cond = nullptr;

		Block* true_target = nullptr,
			 * false_target = nullptr;

		BranchCond()						{ value_type = ValueType_BranchCond; }

		void print();

		static bool check_class(Value* v)	{ return v->value_type == ValueType_BranchCond; }
	};
}