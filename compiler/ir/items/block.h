#pragma once

#include <ir/instructions/instruction.h>

namespace ir
{
	struct Block
	{
	private:
		void add_instruction(Instruction* instruction);

	public:

		std::vector<Instruction*> instructions;
		struct Prototype* parent = nullptr;
		int index = -1;

		~Block();

		void print();

		struct BinaryOp* binary_op(Value* lhs, BinOpType op, Value* rhs);
		struct Call* call(Prototype* callee, const std::vector<Value*>& params);
		struct Cast* cast(Value* value, Type target_type);
		struct Load* load(Value* ptr);
		struct Store* store(Value* ptr, Value* value);
		struct Return* ret(Value* value);
		struct StackAlloc* stackalloc(Type type);
		struct UnaryOp* unary_op(UnaryOpType op, Value* v);
	};
}