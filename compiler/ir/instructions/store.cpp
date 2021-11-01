#include <defs.h>

#include <lexer/lexer.h>

#include "store.h"

#include <ir/ir.h>

void ir::Store::print()
{
	PRINT_INSTRUCTION(1,
		Green, "store ",
		Blue, ptr->type.full_str(),
		White, " ",
		Yellow, ptr->str(), 
		White, ", ",
		Blue, v->type.full_str(),
		White, " ",
		Yellow, v->str());
}