#include <defs.h>

#include <lexer/lexer.h>

#include "load.h"

#include <ir/ir.h>

void ir::Load::print()
{
	PRINT_INSTRUCTION(1,
		Yellow, *v->ir_name, White, " = ",
		Green, "load ",
		Blue, STRIFY_TYPE(v->type.type),
		White, v->type.indirection, White, ", ",
		Blue, STRIFY_TYPE(v1->type.type),
		White, v1->type.indirection, White, " ",
		Yellow, *v1->ir_name);
}