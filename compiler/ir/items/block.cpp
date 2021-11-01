#include <defs.h>

#include "block.h"

#include <ir/items/prototype.h>

#include <ir/instructions/call.h>
#include <ir/instructions/cast.h>
#include <ir/instructions/load.h>
#include <ir/instructions/store.h>
#include <ir/instructions/return.h>
#include <ir/instructions/stack_alloc.h>
#include <ir/instructions/unary_op.h>
#include <ir/instructions/binary_op.h>

namespace ir
{
	void Block::add_instruction(Instruction* instruction)
	{
		instructions.push_back(instruction);
		parent->on_new_instruction(instruction);
	}

	Block::~Block()
	{
		for (auto instruction : instructions)
			_FREE(instruction);
	}

	void Block::print()
	{
		PRINT(Grey, "block_" + std::to_string(index) + ":");

		for (auto instruction : instructions)
			instruction->print();
	}

	BinaryOp* Block::binary_op(Value* lhs, BinOpType op, Value* rhs)
	{
		return nullptr;
	}

	Call* Block::call(Prototype* callee, const std::vector<Value*>& params)
	{
		auto call = _ALLOC(Call);
		call->callee = callee;
		call->args = params;
		call->type = callee->return_type;

		add_instruction(call);

		return call;
	}

	Cast* Block::cast(Value* value, Type target_type)
	{
		auto cast = _ALLOC(Cast);
		cast->source = value;
		cast->type = target_type;

		add_instruction(cast);

		return cast;
	}

	Load* Block::load(Value* ptr)
	{
		auto load = _ALLOC(Load);
		load->ptr = ptr;
		load->type = ptr->type.deref();

		add_instruction(load);

		return load;
	}

	Store* Block::store(Value* ptr, Value* value)
	{
		auto store = _ALLOC(Store);
		store->ptr = ptr;
		store->v = value;
		store->type = Type{ Type_Void, 0 };

		add_instruction(store);

		return store;
	}

	Return* Block::ret(Value* value)
	{
		auto ret = _ALLOC(Return);
		ret->v = value;
		ret->type = Type{ Type_Void, 0 };

		add_instruction(ret);

		return ret;
	}

	StackAlloc* Block::stackalloc(Type type)
	{
		auto stackalloc = _ALLOC(StackAlloc);
		stackalloc->type = type.ref();

		add_instruction(stackalloc);

		return stackalloc;
	}

	UnaryOp* Block::unary_op(UnaryOpType op, Value* v)
	{
		auto unary_op = _ALLOC(UnaryOp);
		unary_op->v = v;
		unary_op->op_type = op;
		unary_op->type = v->type;

		add_instruction(unary_op);

		return unary_op;
	}
}