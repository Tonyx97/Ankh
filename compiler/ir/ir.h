#pragma once

#include <ast/ast.h>

#include "instructions/instruction.h"
#include "instructions/stack_alloc.h"
#include "instructions/cast.h"
#include "instructions/store.h"
#include "instructions/load.h"
#include "instructions/binary_op.h"
#include "instructions/unary_op.h"
#include "instructions/call.h"
#include "instructions/return.h"

#include "items/block.h"
#include "items/body.h"
#include "items/prototype.h"
#include "items/value.h"

namespace ir
{
	struct Context
	{
		std::unordered_map<std::string, Prototype*> prototypes;

		Prototype* pt = nullptr;

		Prototype* find_prototype(const std::string& name)
		{
			auto it = prototypes.find(name);
			return it != prototypes.end() ? it->second : nullptr;
		}
	};
}

class IR
{
private:

	ir::Context ctx {};

	ast::AST* ast = nullptr;

public:

	~IR();

	void run();
	void print();
	void add_prototype(ir::Prototype* prototype);

	// statements

	ir::Prototype* generate_prototype(ast::Prototype* ast_prototype);
	ir::Body* generate_body(ast::StmtBody* ast_body);

	// expressions

	ir::ItemBase* generate_expr(ast::Expr* expr);
	ir::ItemBase* generate_expr_decl(ast::ExprDecl* expr);
	ir::ItemBase* generate_expr_cast(ast::ExprCast* expr);
	ir::ItemBase* generate_expr_id(ast::ExprId* expr);
	ir::ItemBase* generate_expr_int_literal(ast::ExprIntLiteral* expr);
	ir::ItemBase* generate_expr_binary_op(ast::ExprBinaryOp* expr);
	ir::ItemBase* generate_expr_unary_op(ast::ExprUnaryOp* expr);
	ir::ItemBase* generate_expr_call(ast::ExprCall* expr);

	ir::Return* generate_return(ast::StmtReturn* ast_return);

	static inline std::string STRIFY_TYPE(ir::TypeID id)
	{
		switch (id)
		{
		case ir::Type_None: return "none";
		case ir::Type_Void: return "void";
#ifdef TURBO_IR_TOOLS	// oof :(
		case ir::Type_i8:	return "u8";
		case ir::Type_i16:	return "u16";
		case ir::Type_i32:	return "u32";
		case ir::Type_i64:	return "u64";
#else
		case ir::Type_i8:	return "i8";
		case ir::Type_i16:	return "i16";
		case ir::Type_i32:	return "i32";
		case ir::Type_i64:	return "i64";
#endif
		}

		return "unknown";
	}

	static inline std::string STRIFY_TYPE(const ir::Type& type)
	{
		return STRIFY_TYPE(type.type);
	}

	static inline std::string STRIFY_BIN_OP(ir::BinOpType id)
	{
		switch (id)
		{
		case ir::BinOpType_AddAssign:
		case ir::BinOpType_Add:			return "add";
		case ir::BinOpType_SubAssign:
		case ir::BinOpType_Sub:			return "sub";
		case ir::BinOpType_MulAssign:
		case ir::BinOpType_Mul:			return "mul";
		case ir::BinOpType_DivAssign:
		case ir::BinOpType_Div:			return "div";
		case ir::BinOpType_ModAssign:
		case ir::BinOpType_Mod:			return "mod";
		case ir::BinOpType_XorAssign:
		case ir::BinOpType_Xor:			return "xor";
		case ir::BinOpType_Equal:		return "cmp eq";
		case ir::BinOpType_NotEqual:	return "cmp ne";
		case ir::BinOpType_Lt:			return "cmp lt";
		case ir::BinOpType_Lte:			return "cmp lte";
		case ir::BinOpType_Gt:			return "cmp gt";
		case ir::BinOpType_Gte:			return "cmp gte";
		case ir::BinOpType_LogicalAnd:	return "and";
		case ir::BinOpType_LogicalOr:	return "or";
		case ir::BinOpType_And:			return "bit and";
		case ir::BinOpType_Or:			return "bit or";
		case ir::BinOpType_Shr:			return "shr";
		case ir::BinOpType_Shl:			return "shl";
		}

		return "unknown_bin_op";
	}

	static inline std::string STRIFY_UNARY_OP(ir::UnaryOpType id)
	{
		switch (id)
		{
		case ir::UnaryOpType_Add:			return "+";
		case ir::UnaryOpType_Sub:			return "neg";
		case ir::UnaryOpType_Mul:			return "deref";
		case ir::UnaryOpType_And:			return "address";
		case ir::UnaryOpType_Not:			return "not";
		case ir::UnaryOpType_Inc:			return "add";
		case ir::UnaryOpType_Dec:			return "neg";
		case ir::UnaryOpType_LogicalNot:	return "not";
		case ir::UnaryOpType_LogicalAnd:	return "and";
		}

		return "unknown_unary_op";
	}
};

inline std::unique_ptr<IR> g_ir;