#include <defs.h>

#include <lexer/lexer.h>

#include "load.h"

#include <ir/ir.h>

void ir::Load::print()
{
	PRINT_INSTRUCTION(1,
		Yellow, *v->ir_name, White, " = ",
		Green, "load ",
		Blue, v->type.str(),
		White, v->type.indirection, White, ", ",
		Blue, v1->type.str(),
		White, v1->type.indirection, White, " ",
		Yellow, *v1->ir_name);
}