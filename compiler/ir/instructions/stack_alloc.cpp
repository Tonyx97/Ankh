#include <defs.h>

#include <lexer/lexer.h>

#include "stack_alloc.h"

#include <ir/ir.h>

void ir::StackAlloc::print()
{
	PRINT_INSTRUCTION(1,
		Yellow, *v->ir_name,
		White, " = ",
		Green, "stackalloc ",
		Blue, IR::STRIFY_TYPE(v->type),
		White, v->type.indirection - 1);
}