#pragma once

#include <ast/ast.h>

#include "items/prototype.h"
#include "items/body.h"

namespace ir
{
	struct PrototypeCtx
	{
		Prototype* pt = nullptr;
	};

	struct GlobalCtx
	{
		std::unordered_map<std::string, Prototype*> prototypes;
	};
}

class IR
{
private:

	ir::GlobalCtx g_ctx {};
	ir::PrototypeCtx p_ctx {};

	ast::AST* ast = nullptr;

public:

	void run();
	void print();
	void add_prototype(ir::Prototype* prototype);

	ir::Prototype* generate_prototype(ast::Prototype* ast_prototype);
	ir::Body* generate_from_body(ast::StmtBody* ast_body);
};

inline std::unique_ptr<IR> g_ir;