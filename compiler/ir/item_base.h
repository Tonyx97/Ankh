#pragma once

#include <ir/item_type.h>

namespace ir
{
	struct Value;

	struct ItemBase
	{
		ItemType base_type = ItemType_None;

		Value* v = nullptr;
	};
}