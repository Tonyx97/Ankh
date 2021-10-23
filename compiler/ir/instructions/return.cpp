#include <defs.h>

#include <lexer/lexer.h>

#include "return.h"

void ir::Return::print()
{
	PRINT_INSTRUCTION(1, Green, "ret ", Blue, "void");

	/*if (op_i)
		PRINT(Yellow, op_i->get_value_str());
	else */
}