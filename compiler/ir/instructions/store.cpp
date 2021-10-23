#include <defs.h>

#include <lexer/lexer.h>

#include "store.h"

#include <ir/ir.h>

void ir::Store::print()
{
	PRINT_INSTRUCTION(1,
		Green, "store ",
		Blue, IR::STRIFY_TYPE(v->type),
		White, v->type.indirection, White, " ",
		Yellow, *v->ir_name, White, ", ",
		Blue, IR::STRIFY_TYPE(v1->type),
		White, v1->type.indirection, White, " ",
		Yellow, *v1->ir_name);
}