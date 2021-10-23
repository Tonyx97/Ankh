#pragma once

#include <ir/item_base.h>

namespace ir
{
	struct Value : public ItemBase
	{
		optional_str name,
					 ir_name;

		std::string data;

		Type type {};

		virtual ~Value() = default;
	};

	struct ValueId : public Value
	{
		ValueId(const Type& type, const optional_str& ir_name, const optional_str& name)
		{
			base_type = ItemType_ValueId;
			v = this;
			v->name = name;
			v->ir_name = ir_name;
			v->type = type;
		}

		static bool check_class(ItemBase* i)	{ return i->base_type == ItemType_ValueId; }
	};

	struct ValueInt : public Value
	{
		Int vi = { 0 };
		
		ValueInt(const Type& type)
		{
			base_type = ItemType_ValueInt;
			v = this;
			v->type = type;
		}

		static bool check_class(ItemBase* i)	{ return i->base_type == ItemType_ValueInt; }
	};
}