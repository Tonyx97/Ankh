#pragma once

#include "instruction.h"

namespace ir
{
	struct BinaryOp : public Instruction
	{
		BinOpType op_type = BinOpType_None;

		Value* lhs = nullptr,
			 * rhs = nullptr;

		BinaryOp()							{ value_type = ValueType_BinOp; }

		void print();

		static bool check_class(Value* v)	{ return v->value_type == ValueType_BinOp; }
	};
}