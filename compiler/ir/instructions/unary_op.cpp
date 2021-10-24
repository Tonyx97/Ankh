#include <defs.h>

#include <lexer/lexer.h>

#include "unary_op.h"

#include <ir/ir.h>

void ir::UnaryOp::print()
{
	PRINT_INSTRUCTION(1,
		Yellow, *v->ir_name,
		White, " = ",
		Green, STRIFY_UNARY_OP(op_type), White, " ",
		Blue, v->type.str(), White, " ",
		Yellow, *v1->ir_name);
}