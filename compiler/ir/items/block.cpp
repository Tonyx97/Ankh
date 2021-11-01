#include <defs.h>

#include "block.h"

#include <ir/items/prototype.h>

#include <ir/instructions/stack_alloc.h>
#include <ir/instructions/load.h>
#include <ir/instructions/store.h>
#include <ir/instructions/cast.h>
#include <ir/instructions/binary_op.h>
#include <ir/instructions/unary_op.h>
#include <ir/instructions/call.h>
#include <ir/instructions/return.h>
#include <ir/instructions/branch.h>
#include <ir/instructions/branch_cond.h>
#include <ir/instructions/phi.h>

namespace ir
{
	void Block::add_instruction(Instruction* instruction)
	{
		instruction->parent = this;

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
		PRINT(Grey, str() + ":");

		for (auto instruction : instructions)
			instruction->print();

		if (instructions.empty())
			PRINT_NL;
	}

	StackAlloc* Block::add_stack_alloc(const Type& type, int size)
	{
		auto stack_alloc = _ALLOC(StackAlloc);

		stack_alloc->type = type.ref();
		stack_alloc->size = size;

		add_instruction(stack_alloc);

		return stack_alloc;
	}

	Load* Block::add_load(Value* ptr)
	{
		auto load = _ALLOC(Load);

		load->ptr = ptr;
		load->type = ptr->type.deref();

		add_instruction(load);

		return load;
	}

	Store* Block::add_store(Value* ptr, Value* value)
	{
		check(ptr->type.deref() == value->type, "Invalid store operation");

		auto store = _ALLOC(Store);

		store->ptr = ptr;
		store->v = value;

		add_instruction(store);

		return store;
	}

	Cast* Block::add_cast(Value* value, const Type& target_type)
	{
		auto cast = _ALLOC(Cast);

		cast->source = value;
		cast->type = target_type;

		add_instruction(cast);

		return cast;
	}

	BinaryOp* Block::add_binary_op(BinOpType op, Value* lhs, Value* rhs)
	{
		check(lhs->type == rhs->type, "Invalid binary operation");

		auto bin_op = _ALLOC(BinaryOp);

		bin_op->op_type = op;
		bin_op->lhs = lhs;
		bin_op->rhs = rhs;
		bin_op->type = lhs->type; // ?

		add_instruction(bin_op);

		return bin_op;
	}

	UnaryOp* Block::add_unary_op(UnaryOpType op, Value* v)
	{
		auto unary_op = _ALLOC(UnaryOp);

		unary_op->v = v;
		unary_op->op_type = op;
		unary_op->type = v->type;

		add_instruction(unary_op);

		return unary_op;
	}

	Call* Block::add_call(Prototype* callee, const std::vector<Value*>& params)
	{
		auto call = _ALLOC(Call);

		call->callee = callee;
		call->args = params;
		call->type = callee->return_type;

		add_instruction(call);

		return call;
	}

	Return* Block::add_ret(Value* value)
	{
		auto ret = _ALLOC(Return);

		ret->v = value;

		add_instruction(ret);

		return ret;
	}

	Branch* Block::add_branch(Block* target)
	{
		auto branch = _ALLOC(Branch);

		branch->target = target;

		add_instruction(branch);

		return branch;
	}

	BranchCond* Block::add_bcond(Value* cond, Block* true_target, Block* false_target)
	{
		check(true_target != false_target, "Invalid branch cond operation");

		auto branch = _ALLOC(BranchCond);

		branch->cond = cond;
		branch->true_target = true_target;
		branch->false_target = false_target;

		add_instruction(branch);

		return branch;
	}

	Phi* Block::add_phi(const Type& type)
	{
		check(type != Type_None, "Invalid branch cond operation");
		
		auto phi = _ALLOC(Phi);

		phi->type = type;

		add_instruction(phi);

		return phi;
	}
}