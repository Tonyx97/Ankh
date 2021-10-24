#include <defs.h>

#include <lexer/lexer.h>

#include "store.h"

#include <ir/ir.h>

void ir::Store::print()
{
	PRINT_INSTRUCTION(1,
		Green, "store ",
		Blue, STRIFY_TYPE(v->type.type),
		White, v->type.indirection, White, " ",
		Yellow, *v->ir_name, White, ", ",
		Blue, STRIFY_TYPE(v1->type.type),
		White, v1->type.indirection, White, " ",
		Yellow, *v1->ir_name);
}