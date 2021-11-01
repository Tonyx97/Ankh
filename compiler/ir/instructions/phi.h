#pragma once

#include "instruction.h"

namespace ir
{
	struct Value;

	struct Phi : public Instruction
	{
		std::vector<std::pair<Value*, Block*>> values;

		Phi()								{ value_type = ValueType_Phi; }

		void print();
		void add_values() {}

		template <typename... A>
		void add_values(Value* v, Block* b, const A&... args)
		{
			values.push_back({ v, b });
			add_values(args...);
		}

		static bool check_class(Value* v)	{ return v->value_type == ValueType_Phi; }
	};
}