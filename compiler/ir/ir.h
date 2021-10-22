#pragma once

#include <ast/ast.h>

#include "items/prototype.h"
#include "items/body.h"

namespace ir
{
	struct IR
	{
		std::vector<Prototype*> prototypes;
	};
}

class IR
{
private:

	ir::IR ir_ctx {};

	ast::AST* tree = nullptr;

public:

	void run();

	ir::Prototype* generate_prototype(ast::Prototype* prototype);
	ir::Body* generate_from_body(ast::StmtBody* body);
};

inline std::unique_ptr<IR> g_ir;