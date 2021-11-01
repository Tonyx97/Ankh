#include <defs.h>

#include <lexer/lexer.h>

#include "branch.h"

#include <ir/ir.h>

void ir::Branch::print()
{
	PRINT_INSTRUCTION(1,
		Green, "branch ",
		Yellow, target->str()
	);
}