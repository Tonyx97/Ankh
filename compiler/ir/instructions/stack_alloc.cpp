#include <defs.h>

#include <lexer/lexer.h>

#include "stack_alloc.h"

#include <ir/items/value.h>

void ir::StackAlloc::print()
{
	PRINT_EX2(1, Yellow, v->name, White, " = ", Green, "stackalloc ", Blue, Lexer::STRIFY_TYPE(v->type));
}