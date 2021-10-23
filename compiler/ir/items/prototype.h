#pragma once

#include "block.h"

namespace ir
{
	struct Instruction;
	struct Body;
	struct Value;
	struct ValueId;
	struct ValueInt;

	struct PrototypeIrCtx
	{
		Block* block = nullptr;
	};

	struct Prototype
	{
		std::vector<Block*> blocks;
		std::vector<Type> params;
		std::vector<Instruction*> returns;
		std::vector<Value*> values,
							id_values,
							int_values;

		std::unordered_map<std::string, Value*> values_map;

		std::string name;

		PrototypeIrCtx ir_ctx {};

		Type ret_type {};

		Block* entry = nullptr;

		Body* body = nullptr;

		Prototype() {}
		~Prototype();

		void print();
		void add_param(Type&& param)			{ params.push_back(param); }

		bool has_blocks() const					{ return !blocks.empty(); }
		bool has_values() const					{ return !values.empty(); }
		bool has_returns() const				{ return !returns.empty(); }

		Block* create_block();
		Block* add_block(Block* block);
		Block* add_new_block();

		Value* find_value(const std::string& name);
		Value* save_value(Value* v);
		ValueId* add_new_value_id(const Type& type, const optional_str& name = {});
		ValueInt* add_new_value_int(const Type& type);

		Return* add_return(Instruction* item);

		template <typename T, typename... A>
		T* create_item(const A&... args)
		{
			check(ir_ctx.block, "There must be an bound block when adding an item");

			return ir_ctx.block->create_item<T>(args...);
		}

		template <typename T, typename... A>
		T* add_item(T* item)
		{
			check(ir_ctx.block, "There must be an bound block when adding an item");

			return ir_ctx.block->add_item<T>(item);
		}

		template <typename T, typename... A>
		T* add_new_item(const A&... args)
		{
			check(ir_ctx.block, "There must be an bound block when adding an item");

			return ir_ctx.block->add_new_item<T>(args...);
		}
	};
}