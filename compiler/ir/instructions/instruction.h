#pragma once

#include "ins_type.h"
#include "base_item.h"

namespace ir
{
	struct Instruction : public ItemBase
	{
		InstructionType base_type = Ins_None;

		virtual void print() = 0;
	};
}