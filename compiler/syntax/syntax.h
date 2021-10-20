#pragma once

#include <ast/ast.h>

namespace syntax
{
	struct PrototypeContext
	{
		std::unordered_map<std::string, Token*> id_types;

		ast::Prototype* fn = nullptr;

		PrototypeContext& operator = (ast::Prototype* prototype)
		{
			id_types.clear();
			fn = prototype;
			return *this;
		}
	};
}

class Syntax
{
private:

	syntax::PrototypeContext prototype_ctx {};

	ast::AST* tree = nullptr;

public:

	Syntax();
	~Syntax();

	void print_ast();
	void run();
	void add_id_type(Token* id, Token* type);
	void set_id_type(Token* id);

	std::vector<ast::Base*> parse_prototype_params_decl();
	std::vector<ast::Expr*> parse_call_params();

	ast::StmtBody* parse_body(ast::StmtBody* body);
	ast::Base* parse_statement();
	ast::Expr* parse_expression();
	ast::Expr* parse_expression_precedence(ast::Expr* lhs, int min_precedence = Token::LOWEST_PRECEDENCE);
	ast::Expr* parse_primary_expression();
	ast::AST* get_ast()			{ return tree; }

	Token* get_id_type(Token* id);
};

inline std::unique_ptr<Syntax> g_syntax;