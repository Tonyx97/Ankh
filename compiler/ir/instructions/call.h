#pragma once

#include "instruction.h"

namespace ir
{
	struct Prototype;

	struct Call : public Instruction
	{
		Prototype* callee = nullptr;
		std::vector<Value*> args;

		Call()									{ value_type = ValueType_Call; }

		void print();

		static bool check_class(Value* v)	{ return v->value_type == ValueType_Call; }
	};
}