#include <defs.h>

#include <lexer/lexer.h>

#include "binary_op.h"

#include <ir/ir.h>

void ir::BinaryOp::print()
{
	PRINT_INSTRUCTION(1,
		Yellow, str(),
		White, " = ",
		Green, STRIFY_BIN_OP(op_type), White, " ",
		Blue, type.str(),
		White, type.indirection, White, " ",
		Yellow, lhs->str(), White, ", ",
		Yellow, rhs->str());
}