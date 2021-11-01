#pragma once

#include <ir/types.h>

namespace ir
{
	enum ValueType
	{
		ValueType_None,
		ValueType_ConstantInt,
		ValueType_Parameter,
		ValueType_BeginInstructionType,
			ValueType_StackAlloc,
			ValueType_Load,
			ValueType_Store,
			ValueType_Cast,
			ValueType_BinOp,
			ValueType_UnaryOp,
			ValueType_Call,
			ValueType_Return,
			ValueType_Branch,
			ValueType_BranchCond,
			ValueType_Gep,
			ValueType_Select,
			ValueType_Phi,
		ValueType_EndInstructionType,
	};

	struct Value
	{
		Type type {};

		ValueType value_type {};

		struct Block* parent = nullptr;

		int index = -1;

		virtual ~Value() = default;

		bool is_void()						{ return type.is_same_type(Type_Void); }

		std::string str() const				{ return "v" + std::to_string(index); }

		static bool check_class(Value* v)	{ return true; }
	};

	struct ValueParam : public Value 
	{
		ValueParam(const Type& type)
		{
			this->value_type = ValueType_Parameter;
			this->type = type;
		}

		static bool check_class(Value* v) { return v->value_type == ValueType_Parameter; }
	};

	struct ValueInt : public Value
	{
		Int integer = { 0 };
		
		ValueInt(const Type& type, Int integer) : integer(integer)
		{
			this->value_type = ValueType_ConstantInt;
			this->type = type;
		}

		static bool check_class(Value* v) { return v->value_type == ValueType_ConstantInt; }
	};
}