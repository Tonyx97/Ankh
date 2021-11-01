#include <defs.h>

#include <lexer/lexer.h>

#include "cast.h"

#include <ir/ir.h>

void ir::Cast::print()
{
	PRINT_INSTRUCTION(1,
		Yellow, str(), White, " = ",
		Green, "cast ",
		Blue, source->type.str(), White, " ",
		Yellow, source->str(), White, " to ",
		Blue, type.str());
}