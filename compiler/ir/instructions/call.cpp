#include <defs.h>

#include <lexer/lexer.h>

#include "call.h"

#include <ir/ir.h>

void ir::Call::print()
{
	/*
	if (prototype->ret_type.type == Type_Void)
	{
		PRINT_INSTRUCTION_NNL(1,
			Green, "call ",
			Blue, prototype->ret_type.str(), White, " ",
			Yellow, prototype->name, White, "(");
	}
	else
	{
		PRINT_INSTRUCTION_NNL(1,
			Yellow, *v->ir_name,
			White, " = ",
			Green, "call ",
			Blue, prototype->ret_type.str(), White, " ",
			Yellow, prototype->name, White, "(");
	}

	fullprint_vec<Value>(White, args, ", ", [](auto arg)
	{
		PRINT_INSTRUCTION_NNL(0,
			Blue, arg->type.str(),
			White, arg->type.indirection, White, " ",
			Yellow, *arg->ir_name);
	});

	PRINT_INSTRUCTION(0, White, ")");
	*/
}