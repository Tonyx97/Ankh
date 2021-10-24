#pragma once

#include <ir/types.h>

namespace ir
{
	struct Value;

	struct ItemBase
	{
		ItemType item_type = ItemType_None;

		Value* v = nullptr;
	};
}