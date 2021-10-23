#pragma once

#include <ir/item_base.h>

namespace ir
{
	struct Instruction : public ItemBase
	{
		virtual void print() = 0;
	};
}