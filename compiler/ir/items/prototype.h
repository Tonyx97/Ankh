#pragma once

#include "block.h"

namespace ir
{
	struct Prototype
	{
		std::string name;

		Type return_type{};
		std::vector<ValueParam*> params;
		
		std::vector<Block*> blocks;

		Block* entry = nullptr;

		int next_value_index = 0;
		int next_block_index = 0;

		void set_value_index(Value* value);
		void on_new_instruction(Instruction* instruction);

		Prototype(const std::string& name, Type return_type, const std::vector<Type>& param_types);
		~Prototype();

		Block* create_block();

		void print();
	};
}