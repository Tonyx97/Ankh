#include <defs.h>

#include <lexer/lexer.h>

#include "return.h"

#include <ir/ir.h>

void ir::Return::print()
{
	if (v->is_void())
	{
		PRINT_INSTRUCTION(1,
			Green, "ret ",
			Blue ,"void"
		);
	}
	else
	{
		PRINT_INSTRUCTION(1,
			Green, "ret ",
			Blue, v->type.full_str(),
			White, " ",
			Yellow, v->str()
		);
	}
}