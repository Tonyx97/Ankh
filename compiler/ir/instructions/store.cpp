#include <defs.h>

#include <lexer/lexer.h>

#include "store.h"

#include <ir/items/value.h>

void ir::Store::print()
{
	PRINT_EX2(1,
		Green, "store ",
		Blue, Lexer::STRIFY_TYPE(v->type), White, " ",
		Yellow, v->name, White, " ",
		Blue, Lexer::STRIFY_TYPE(v1->type), White, " ",
		Yellow, v1->name);
}