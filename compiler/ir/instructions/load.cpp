#include <defs.h>

#include <lexer/lexer.h>

#include "load.h"

#include <ir/ir.h>

void ir::Load::print()
{
	PRINT_INSTRUCTION(1,
		Yellow, *v->ir_name, White, " = ",
		Green, "load ",
		Blue, IR::STRIFY_TYPE(v->type),
		White, v->type.indirection, White, ", ",
		Blue, IR::STRIFY_TYPE(v1->type),
		White, v1->type.indirection, White, " ",
		Yellow, *v1->ir_name);
}