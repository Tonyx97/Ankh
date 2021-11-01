#include <defs.h>

#include <lexer/lexer.h>

#include "phi.h"

#include <ir/ir.h>

void ir::Phi::print()
{
	PRINT_INSTRUCTION_NNL(1,
		Yellow, str(), White, " = ",
		Green, "phi ",
		Blue, type.str(),
		White, type.indirection, White, " "
	);

	PRINT_NNL(White, "[");

	fullprint_vec_obj<std::pair<Value*, Block*>>(White, values, ", ", [](const std::pair<Value*, Block*>& e)
	{
		PRINT_INSTRUCTION_NNL(0,
			Grey, e.second->str(), White, ": ",
			Yellow, e.first->str());
	});

	PRINT(White, "]");
}