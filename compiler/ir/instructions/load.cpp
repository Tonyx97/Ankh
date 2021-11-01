#include <defs.h>

#include <lexer/lexer.h>

#include "load.h"

#include <ir/ir.h>

void ir::Load::print()
{
	PRINT_INSTRUCTION(1,
		Yellow, str(), White, " = ",
		Green, "load ",
		Blue, type.full_str(),
		White, ", ",
		Blue, ptr->type.full_str(),
		White, " ",
		Yellow, ptr->str()
	);
}