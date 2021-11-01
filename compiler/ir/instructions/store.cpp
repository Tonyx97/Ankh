#include <defs.h>

#include <lexer/lexer.h>

#include "store.h"

#include <ir/ir.h>

void ir::Store::print()
{
	PRINT_INSTRUCTION(1,
		Green, "store ",
		Blue, ptr->type.str(),
		White, ptr->type.indirection, White, " ",
		Yellow, ptr->str(), 
		White, ", ",
		Blue, v->type.str(),
		White, v->type.indirection, White, " ",
		Yellow, v->str());
}