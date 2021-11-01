#pragma once

#include "instruction.h"

namespace ir
{
	struct UnaryOp : public Instruction
	{
		Value* v = nullptr;

		UnaryOpType op_type = UnaryOpType_None;

		UnaryOp()							{ value_type = ValueType_UnaryOp; }

		void print();

		static bool check_class(Value* v)	{ return v->value_type == ValueType_UnaryOp; }
	};
}