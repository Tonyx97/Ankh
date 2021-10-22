#pragma once

#include "block.h"

namespace ir
{
	struct Body;

	struct PrototypeIrCtx
	{
		Block* block = nullptr;
	};

	struct Prototype
	{
		std::vector<Block*> blocks;
		std::vector<TokenIR> params;

		std::string name;

		PrototypeIrCtx ir_ctx {};

		TokenIR type {};

		Block* entry = nullptr;

		Body* body = nullptr;

		Prototype() {}
		~Prototype();

		void print();
		void add_param(TokenIR param)			{ params.push_back(param); }

		bool is_empty() const					{ return blocks.empty(); }

		Block* create_block();
		Block* add_block(Block* block);
		Block* add_new_block();

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