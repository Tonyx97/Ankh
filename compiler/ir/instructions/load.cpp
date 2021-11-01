#include <defs.h>

#include <lexer/lexer.h>

#include "load.h"

#include <ir/ir.h>

void ir::Load::print()
{
	PRINT_INSTRUCTION(1,
		Yellow, str(), White, " = ",
		Green, "load ",
		Blue, type.str(),
		White, type.indirection, White, ", ",
		Blue, ptr->type.str(),
		White, type.indirection, White, " ",
		Yellow, ptr->str()
	);
}