#include <defs.h>

#include <lexer/lexer.h>

#include "binary_op.h"

#include <ir/ir.h>

void ir::BinaryOp::print()
{
	/*
	PRINT_INSTRUCTION(1,
		Yellow, *v->ir_name,
		White, " = ",
		Green, STRIFY_BIN_OP(op_type), White, " ",
		Blue, v->type.str(),
		White, v->type.indirection, White, " ",
		Yellow, *v1->ir_name, White, ", ",
		Yellow, *v2->ir_name);
		*/
}