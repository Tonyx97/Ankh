#pragma once

#include "instruction.h"

namespace ir
{
	struct Call : public Instruction
	{
		std::vector<Value*> args;

		struct Prototype* callee = nullptr;

		Call()								{ value_type = ValueType_Call; }

		void print();

		static bool check_class(Value* v)	{ return v->value_type == ValueType_Call; }
	};
}