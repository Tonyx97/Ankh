#include <defs.h>

#include <lexer/lexer.h>

#include "return.h"

#include <ir/ir.h>

void ir::Return::print()
{
	if (v)
		PRINT_INSTRUCTION(1,
			Green, "ret ",
			Blue, IR::STRIFY_TYPE(v->type), White, " ",
			Yellow, *v->ir_name);
	else
		PRINT_INSTRUCTION(1,
			Green, "ret ",
			Blue, "void");
}