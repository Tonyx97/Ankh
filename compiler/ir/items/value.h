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

		Value* param_value = nullptr;

		virtual ~Value() = default;

		Value* clone();
	};

	struct ValueId : public Value
	{
		ValueId(const Type& type, const optional_str& ir_name, const optional_str& name)
		{
			item_type = ItemType_ValueId;
			v = this;
			v->name = name;
			v->ir_name = ir_name;
			v->type = type;
		}

		static bool check_class(ItemBase* i)	{ return i->item_type == ItemType_ValueId; }
	};

	struct ValueInt : public Value
	{
		Int vi = { 0 };
		
		ValueInt(const Type& type, const optional_str& name = {})
		{
			item_type = ItemType_ValueInt;
			v = this;
			v->name = v->ir_name = name;
			v->type = type;
		}

		static bool check_class(ItemBase* i)	{ return i->item_type == ItemType_ValueInt; }
	};
}