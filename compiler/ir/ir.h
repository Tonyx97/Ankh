#pragma once

#include <ast/ast.h>

#include "instructions/instruction.h"
#include "instructions/stack_alloc.h"
#include "instructions/cast.h"
#include "instructions/store.h"
#include "instructions/load.h"
#include "instructions/binary_op.h"
#include "instructions/unary_op.h"
#include "instructions/call.h"
#include "instructions/return.h"

#include "items/block.h"
#include "items/prototype.h"
#include "items/value.h"

namespace ir
{
	struct Context
	{
		std::unordered_map<std::string, Prototype*> prototypes;

		Prototype* pt = nullptr;

		Prototype* find_prototype(const std::string& name)
		{
			auto it = prototypes.find(name);
			return it != prototypes.end() ? it->second : nullptr;
		}
	};
}