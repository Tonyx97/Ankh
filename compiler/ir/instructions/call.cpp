#include <defs.h>

#include <lexer/lexer.h>

#include "call.h"

#include <ir/ir.h>

void ir::Call::print()
{
	if (callee->return_type == Type_Void)
	{
		PRINT_INSTRUCTION_NNL(1,
			Green, "call ",
			Blue, callee->return_type.str(), White, " ",
			Yellow, callee->name, White, "(");
	}
	else
	{
		PRINT_INSTRUCTION_NNL(1,
			Yellow, str(),
			White, " = ",
			Green, "call ",
			Blue, callee->return_type.str(), White, " ",
			Yellow, callee->name, White, "(");
	}

	fullprint_vec<Value>(White, args, ", ", [](Value* arg)
	{
		PRINT_INSTRUCTION_NNL(0,
			Blue, arg->type.str(),
			White, arg->type.indirection, White, " ",
			Yellow, arg->str());
	});

	PRINT_INSTRUCTION(0, White, ")");
}