#include <defs.h>

#include <lexer/lexer.h>

#include "call.h"

#include <ir/ir.h>

void ir::Call::print()
{
	PRINT_INSTRUCTION_NNL(1,
		Yellow, *v->ir_name,
		White, " = ",
		Green, "call ",
		Blue, IR::STRIFY_TYPE(prototype->ret_type), White, " ",
		Yellow, prototype->name, White, "(");

	fullprint_vec<Value>(White, args, ", ", [](auto arg)
	{
		PRINT_INSTRUCTION_NNL(0,
			Blue, IR::STRIFY_TYPE(arg->type),
			White, arg->type.indirection, White, " ",
			Yellow, *arg->ir_name);
	});

	PRINT_INSTRUCTION(0, White, ")");
}