#pragma once

#include "block.h"

namespace ir
{
	struct type_and_constant_int_hash
	{
		template <typename Tx, typename Ty>
		size_t operator () (const std::pair<Tx, Ty>& p) const
		{
			auto h1 = std::hash<Tx>{}(p.first);
			auto h2 = std::hash<Ty>{}(p.second);

			size_t seed = 0ull;

			seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

			return seed;
		}
	};

	using type_and_constant = std::pair<Type, Int>;

	struct Prototype
	{
		std::vector<ValueParam*> params;
		std::vector<Block*> blocks;
		std::unordered_map<type_and_constant, Value*, type_and_constant_int_hash> constants;

		std::string name;

		Type return_type {};

		Block* entry = nullptr;

		int next_value_index = 0,
			next_block_index = 0;

		Prototype(const std::string& name, const Type& return_type, const std::vector<Type>& param_types);
		~Prototype();
		
		void set_value_index(Value* value);
		void on_new_instruction(Instruction* instruction);
		void print();

		Block* create_block();
	};
}