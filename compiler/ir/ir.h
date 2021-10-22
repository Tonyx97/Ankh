#pragma once

#include <ast/ast.h>

#include "instructions/instruction.h"
#include "instructions/return.h"

#include "items/block.h"
#include "items/body.h"
#include "items/prototype.h"

namespace ir
{
	struct Context
	{
		std::unordered_map<std::string, Prototype*> prototypes;

		Prototype* pt = nullptr;
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

	ir::Prototype* generate_prototype(ast::Prototype* ast_prototype);
	ir::Body* generate_body(ast::StmtBody* ast_body);

	ir::Return* generate_return(ast::StmtReturn* ast_return);
};

inline std::unique_ptr<IR> g_ir;