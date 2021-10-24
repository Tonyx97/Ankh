#include <defs.h>

#include <lexer/lexer.h>

#include "cast.h"

#include <ir/ir.h>

void ir::Cast::print()
{
	PRINT_INSTRUCTION(1,
		Yellow, *v->ir_name, White, " = ",
		Green, "cast ",
		Blue, STRIFY_TYPE(v1->type.type), White, " ",
		Yellow, *v1->ir_name, White, " to ",
		Blue, STRIFY_TYPE(v->type.type));
}