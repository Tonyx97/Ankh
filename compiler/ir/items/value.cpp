#include <defs.h>

#include <lexer/lexer.h>

#include "value.h"

namespace ir
{
	Value* Value::clone()
	{
		Value* new_value = nullptr;

		if (auto value_id = rtti::cast<ValueId>(this))
		{
			auto new_value_id = _ALLOC(ValueId, type, ir_name, name);

			new_value = new_value_id;
		}
		else if (auto value_int = rtti::cast<ValueInt>(this))
		{
			auto new_value_id = _ALLOC(ValueInt, type);

			new_value_id->vi = value_int->vi;

			new_value = new_value_id;
		}

		new_value->name = name;
		new_value->ir_name = ir_name;
		new_value->data = data;
		new_value->type = type;

		return new_value;
	}
}