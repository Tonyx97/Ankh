#include <defs.h>

#include <lexer/lexer.h>
#include <ir/ir.h>

#include "prototype.h"
#include "value.h"

namespace ir
{
	Prototype::Prototype(const std::string& name, const Type& return_type, const std::vector<Type>& param_types) : name(name), return_type(return_type)
	{
		for (auto param_type : param_types)
		{
			auto value = _ALLOC(ValueParam, param_type);
			set_value_index(value);
			params.push_back(value);
		}
	}

	Prototype::~Prototype()
	{
		for (auto param : params) _FREE(param);
		for (auto block : blocks) _FREE(block);
	}

	void Prototype::set_value_index(Value* value)
	{
		check(value->index == -1, "Index already set");

		value->index = next_value_index++;
	}

	void Prototype::on_new_instruction(Instruction* instruction)
	{
		if (instruction->type != Type_Void)
			set_value_index(instruction);
	}

	void Prototype::print()
	{
		PRINT_INSTRUCTION_NNL(0,
			Blue, return_type.str(), 
			White, return_type.indirection,
			White, " ",
			Yellow, name,
			White, "("
		);

		fullprint_vec<Value>(White, params, ", ", [](Value* param)
		{
			PRINT_INSTRUCTION_NNL(0,
				Blue, param->type.str(),
				White, param->type.indirection, White, " ",
				Yellow, param->str());
		});

		PRINT_INSTRUCTION(0, White, ")");
		PRINT_INSTRUCTION(0, White, "{");

		for (auto block : blocks)
			block->print();

		PRINT_INSTRUCTION(0, White, "}");
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
}