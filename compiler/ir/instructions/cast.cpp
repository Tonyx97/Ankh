#include <defs.h>

#include <lexer/lexer.h>

#include "cast.h"

#include <ir/items/value.h>

void ir::Cast::print()
{
	PRINT_EX2(1,
		Yellow, v->name, White, " = ",
		Green, "cast ",
		Blue, Lexer::STRIFY_TYPE(v1->type), White, " ",
		Yellow, v1->name, White, " to ",
		Blue, Lexer::STRIFY_TYPE(v->type));
}