#pragma once

#include "instruction.h"

namespace ir
{
	struct Return : public Instruction
	{
		Instruction* op_i = nullptr;

		TokenIR type {};

		Return()								{ base_type = Ins_Return; }

		void print();

		static bool check_class(Instruction* i) { return i->base_type == Ins_Return; }
	};
}