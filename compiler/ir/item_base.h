#pragma once

#include <ir/types.h>

namespace ir
{
	struct Value;

	struct ItemBase
	{
		ItemType base_type = ItemType_None;

		Value* v = nullptr;
	};
}