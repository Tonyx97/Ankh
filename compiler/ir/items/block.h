#pragma once

#include <ir/instructions/instruction.h>

namespace ir
{
	struct Prototype;

	struct Block
	{
	private:

		void add_instruction(Instruction* instruction);

	public:

		std::vector<Instruction*> instructions;

		Prototype* parent = nullptr;

		int index = -1;

		~Block();

		void print();

		struct StackAlloc* add_stack_alloc(const Type& type, int size = 1);
		struct Load* add_load(Value* ptr);
		struct Store* add_store(Value* ptr, Value* value);
		struct Cast* add_cast(Value* value, const Type& target_type);
		struct BinaryOp* add_binary_op(BinOpType op, Value* lhs, Value* rhs);
		struct UnaryOp* add_unary_op(UnaryOpType op, Value* v);
		struct Call* add_call(Prototype* callee, const std::vector<Value*>& params);
		struct Return* add_ret(Value* value);
		struct Branch* add_branch(Block* target);
		struct BranchCond* add_bcond(Value* cond, Block* true_target, Block* false_target);
		struct Phi* add_phi(const Type& type);

		std::string str() const { return "block_" + std::to_string(index); }
	};
}