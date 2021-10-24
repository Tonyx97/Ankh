#pragma once

union Int
{
	uint64_t u64;
	int64_t i64;

	uint32_t u32;
	int32_t i32;

	uint16_t u16;
	int16_t i16;

	uint8_t u8;
	int8_t i8;

	template <typename T>
	Int& operator = (T v) { u64 = v; return *this; }
};

enum TypeFlag : unsigned __int64
{
	TypeFlag_None = 0ull,
	TypeFlag_Op = (1ull << 0),
	TypeFlag_Keyword = (1ull << 1),
	TypeFlag_KeywordType = (1ull << 2),
	TypeFlag_StaticValue = (1ull << 3),
	TypeFlag_Unsigned = (1ull << 4),
	TypeFlag_Assignation = (1ull << 5),
	TypeFlag_Id = (1ull << 6),
};

enum BinOpType
{
	BinOpType_None,
	BinOpType_AddAssign,
	BinOpType_Add,
	BinOpType_SubAssign,
	BinOpType_Sub,
	BinOpType_MulAssign,
	BinOpType_Mul,
	BinOpType_Indirection = BinOpType_Mul,
	BinOpType_DivAssign,
	BinOpType_Div,
	BinOpType_ModAssign,
	BinOpType_Mod,
	BinOpType_AndAssign,
	BinOpType_And,
	BinOpType_OrAssign,
	BinOpType_Or,
	BinOpType_XorAssign,
	BinOpType_Xor,
	BinOpType_ShrAssign,
	BinOpType_Shr,
	BinOpType_ShlAssign,
	BinOpType_Shl,
	BinOpType_Equal,
	BinOpType_NotEqual,
	BinOpType_Lt,
	BinOpType_Lte,
	BinOpType_Gt,
	BinOpType_Gte,
	BinOpType_LogicalAnd,
	BinOpType_LogicalOr,
};

enum UnaryOpType
{
	UnaryOpType_None,
	UnaryOpType_Add,
	UnaryOpType_Sub,
	UnaryOpType_Mul,
	UnaryOpType_And,
	UnaryOpType_Not,
	UnaryOpType_Inc,
	UnaryOpType_Dec,
	UnaryOpType_LogicalNot,
	UnaryOpType_LogicalAnd,
};

enum TypeID
{
	Type_None,
	Type_Void,
	Type_i8,
	Type_i16,
	Type_i32,
	Type_i64
};

static inline std::string STRIFY_TYPE(TypeID id)
{
	switch (id)
	{
	case Type_None: return "none";
	case Type_Void: return "void";
#ifdef TURBO_IR_TOOLS	// oof :(
	case Type_i8:	return "u8";
	case Type_i16:	return "u16";
	case Type_i32:	return "u32";
	case Type_i64:	return "u64";
#else
	case Type_i8:	return "i8";
	case Type_i16:	return "i16";
	case Type_i32:	return "i32";
	case Type_i64:	return "i64";
#endif
	}

	return "unknown";
}

static inline std::string STRIFY_BIN_OP(BinOpType id)
{
	switch (id)
	{
	case BinOpType_AddAssign:
	case BinOpType_Add:			return "add";
	case BinOpType_SubAssign:
	case BinOpType_Sub:			return "sub";
	case BinOpType_MulAssign:
	case BinOpType_Mul:			return "mul";
	case BinOpType_DivAssign:
	case BinOpType_Div:			return "div";
	case BinOpType_ModAssign:
	case BinOpType_Mod:			return "mod";
	case BinOpType_XorAssign:
	case BinOpType_Xor:			return "xor";
	case BinOpType_Equal:		return "cmp eq";
	case BinOpType_NotEqual:	return "cmp ne";
	case BinOpType_Lt:			return "cmp lt";
	case BinOpType_Lte:			return "cmp lte";
	case BinOpType_Gt:			return "cmp gt";
	case BinOpType_Gte:			return "cmp gte";
	case BinOpType_LogicalAnd:	return "and";
	case BinOpType_LogicalOr:	return "or";
	case BinOpType_And:			return "bit and";
	case BinOpType_Or:			return "bit or";
	case BinOpType_Shr:			return "shr";
	case BinOpType_Shl:			return "shl";
	}

	return "unknown_bin_op";
}

static inline std::string STRIFY_UNARY_OP(UnaryOpType id)
{
	switch (id)
	{
	case UnaryOpType_Add:			return "+";
	case UnaryOpType_Sub:			return "neg";
	case UnaryOpType_Mul:			return "deref";
	case UnaryOpType_And:			return "address";
	case UnaryOpType_Not:			return "not";
	case UnaryOpType_Inc:			return "add";
	case UnaryOpType_Dec:			return "neg";
	case UnaryOpType_LogicalNot:	return "not";
	case UnaryOpType_LogicalAnd:	return "and";
	}

	return "unknown_unary_op";
}