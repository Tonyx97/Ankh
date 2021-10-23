#pragma once

#include <ir/item_base.h>

namespace ir
{
	struct Value : public ItemBase
	{
		std::string name;

		std::string data;

		TokenIR type {};
	};

	struct ValueId : public Value
	{
		ValueId()								{ base_type = ItemType_ValueId; }

		static bool check_class(ItemBase* i)	{ return i->base_type == ItemType_ValueId; }
	};

	struct ValueInt : public Value
	{
		Int vi = { 0 };
		
		ValueInt()								{ base_type = ItemType_ValueInt; }

		static bool check_class(ItemBase* i)	{ return i->base_type == ItemType_ValueInt; }
	};
}