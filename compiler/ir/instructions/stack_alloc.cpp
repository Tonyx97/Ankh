#include <defs.h>

#include <lexer/lexer.h>

#include "stack_alloc.h"

#include <ir/ir.h>

void ir::StackAlloc::print()
{
	PRINT_INSTRUCTION(1,
		Yellow, str(),
		White, " = ",
		Green, "stackalloc ",
		Blue, type.deref().full_str()
	);
}