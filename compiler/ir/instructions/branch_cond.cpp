#include <defs.h>

#include <lexer/lexer.h>

#include "branch_cond.h"

#include <ir/ir.h>

void ir::BranchCond::print()
{
	PRINT_INSTRUCTION(1,
		Green, "bcond ",
		Blue, cond->type.str(),
		White, cond->type.indirection, White, " ",
		Yellow, cond->str(),
		White, ", ",
		Grey, true_target->str(),
		White, ", ",
		Grey, false_target->str()
	);
}