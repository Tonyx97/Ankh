#include <defs.h>

#include <lexer/lexer.h>

#include <ir/instructions/return.h>

#include "prototype.h"
#include "body.h"
#include "value.h"

namespace ir
{
	Prototype::~Prototype()
	{
		for (auto block : blocks)
			_FREE(block);

		_FREE(body);
	}

	void Prototype::print()
	{
		PRINT_NNL(White, "{} {}(", Lexer::STRIFY_TYPE(type), name);

		/*dbg::print_vec<PrototypeParam>(White, prototype->params, ", ", [](auto stmt)
		{
			return Lexer::STRIFY_TYPE(stmt->type) + " " + stmt->name;
		});*/

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

	ValueId* Prototype::add_new_value_id()
	{
		auto v = _ALLOC(ValueId);

		v->name = "v" + std::to_string(values.size());

		values.push_back(v);

		return v;
	}

	ValueInt* Prototype::add_new_value_int()
	{
		auto v = _ALLOC(ValueInt);

		values.push_back(v);

		return v;
	}
}