#include <defs.h>

#include <lexer/lexer.h>
#include <ir/ir.h>

#include "prototype.h"
#include "value.h"

namespace ir
{
	void Prototype::set_value_index(Value* value)
	{
		check(value->index == -1, "Index already set.");

		value->index = next_value_index++;
	}

	void Prototype::on_new_instruction(Instruction* instruction)
	{
		if (!instruction->type.is_same_type(Type_Void))
		{
			set_value_index(instruction);
		}
	}

	Prototype::Prototype(const std::string& name, Type return_type, const std::vector<Type>& param_types)
	{
		this->name = name;
		this->return_type = return_type;

		for (auto param_type : param_types)
		{
			auto value = _ALLOC(ValueParam, param_type);
			set_value_index(value);
			params.push_back(value);
		}
	}

	Prototype::~Prototype()
	{
		for (auto param : params)
			_FREE(param);

		for (auto block : blocks)
			_FREE(block);
	}

	Block* Prototype::create_block()
	{
		auto block = _ALLOC(Block);

		block->index = next_block_index++;
		block->parent = this;

		if (!entry)
			entry = block;

		blocks.push_back(block);

		return block;
	}

	void Prototype::print()
	{
		PRINT_INSTRUCTION_NNL(0,
			Blue, return_type.full_str(), 
			White, " ",
			Yellow, name,
			White, "("
		);

		bool need_comma = false;
		for (auto param : params)
		{
			if (need_comma)
				PRINT_INSTRUCTION_NNL(0, White, ", ");

			PRINT_INSTRUCTION_NNL(0,
				Blue, param->type.full_str(),
				White, " ",
				Yellow, param->str());

			need_comma = true;
		}

		PRINT_INSTRUCTION(0, White, ")");
		PRINT_INSTRUCTION(0, White, "{");

		bool need_newline = false;
		for (auto block : blocks)
		{
			if (need_newline)
				PRINT_INSTRUCTION(0);

			block->print();

			need_newline = true;
		}

		PRINT_INSTRUCTION(0, White, "}");
	}
}