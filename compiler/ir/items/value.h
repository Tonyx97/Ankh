#pragma once

#include <ir/item_base.h>

namespace ir
{
	enum ValueType
	{
		ValueType_None,
		ValueType_ConstantInt,
		ValueType_Parameter,
		ValueType_BeginInstructionType,
		ValueType_Cast,
		ValueType_BinOp,
		ValueType_UnaryOp,
		ValueType_Call,
		ValueType_StackAlloc,
		ValueType_Store,
		ValueType_Load,
		ValueType_Block,
		ValueType_BranchCond,
		ValueType_Branch,
		ValueType_Return,
		ValueType_Phi,
		// TODO: GetElementPointer
		//       Select
		ValueType_EndInstructionType,
	};

	struct Value
	{
		Type type{};
		ValueType value_type{};

		bool is_void() { return type.is_same_type(Type_Void); }

		int index = -1;

		std::string str() const { return "v" + std::to_string(index); }

		virtual ~Value() = default;

		static bool check_class(Value* v) { return true; }
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
		
		ValueInt(const Type& type, Int integer)
		{
			this->value_type = ValueType_ConstantInt;
			this->type = type;
			this->integer = integer;
		}

		static bool check_class(Value* v) { return v->value_type == ValueType_ConstantInt; }
	};
}