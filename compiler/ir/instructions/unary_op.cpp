#include <defs.h>

#include <lexer/lexer.h>

#include "unary_op.h"

#include <ir/ir.h>

void ir::UnaryOp::print()
{
	PRINT_INSTRUCTION(1,
		Yellow, str(),
		White, " = ",
		Green, STRIFY_UNARY_OP(op_type), White, " ",
		Blue, type.str(),
		White, type.indirection, White, " ",
		Yellow, v->str());
}