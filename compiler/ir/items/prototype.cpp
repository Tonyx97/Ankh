#include <defs.h>

#include <lexer/lexer.h>
#include <ir/ir.h>

#include <ir/instructions/return.h>

#include "prototype.h"
#include "body.h"
#include "value.h"

namespace ir
{
	Prototype::~Prototype()
	{
		for (auto value : values)
			_FREE(value);

		for (auto block : blocks)
			_FREE(block);

		_FREE(body);
	}

	void Prototype::print()
	{
		PRINT_NNL(White, "{} {}(", IR::STRIFY_TYPE(ret_type), name);

		fullprint_vec<Value>(White, params, ", ", [](auto param)
		{
			PRINT_INSTRUCTION_NNL(0,
				Blue, IR::STRIFY_TYPE(param->type),
				White, param->type.indirection - 1, White, " ",
				Yellow, *param->ir_name);
		});

		if (has_blocks())
		{
			PRINT(White, ")");

			PRINT(White, "{{");

			for (auto block : blocks)
				block->print();

			PRINT(White, "}}");
		}
		else PRINT(White, ") {{}}");

		PRINT_NL;
	}

	Block* Prototype::create_block()
	{
		return (ir_ctx.block = _ALLOC(Block));
	}

	Block* Prototype::add_block(Block* block)
	{
		if (!has_blocks())
		{
			entry = block;
			entry->name = "entry";
			entry->flags |= BlockFlag_Entry;
		}
		else block->name = "block_" + std::to_string(blocks.size());

		blocks.push_back(block);

		return block;
	}

	Block* Prototype::add_new_block()
	{
		return add_block(create_block());
	}

	Value* Prototype::find_value(const std::string& name)
	{
		auto it = values_map.find(name);
		return it != values_map.end() ? it->second : nullptr;
	}

	Value* Prototype::save_value(Value* v)
	{
		if (rtti::cast<ValueId>(v))	 id_values.push_back(v);
		if (rtti::cast<ValueInt>(v)) int_values.push_back(v);

		values.push_back(v);

		if (v->name.has_value())
			values_map.insert({ v->name.value(), v });

		return v;
	}

	ValueId* Prototype::add_parameter(const Type& type, const optional_str& name)
	{
		auto param = add_new_value_id(type, name);

		params.push_back(param);

		return param;
	}

	ValueId* Prototype::add_new_value_id(const Type& type, const optional_str& name)
	{
		auto v = _ALLOC(ValueId, type, "v" + std::to_string(id_values.size()), name);

		return rtti::cast<ValueId>(save_value(v));
	}

	ValueInt* Prototype::add_new_value_int(const Type& ir_type)
	{
		return rtti::cast<ValueInt>(save_value(_ALLOC(ValueInt, ir_type)));
	}

	Return* Prototype::add_return(Instruction* item)
	{
		returns.push_back(item);
		return rtti::cast<Return>(add_item(item));
	}
}