#pragma once

#include <ast/ast.h>

class Syntax
{
private:

	ast::AST* tree = nullptr;

	ast::Prototype* curr_prototype = nullptr;

public:

	Syntax();
	~Syntax();

	void print_ast();
	void run();
	void optimize_and_fix();

	std::vector<ast::Base*> parse_prototype_params_decl();
	std::vector<ast::Expr*> parse_call_params();

	ast::StmtBody* parse_body(ast::StmtBody* body);
	ast::Base* parse_statement();
	ast::Expr* parse_expression();
	ast::Expr* parse_expression_precedence(ast::Expr* lhs, int min_precedence = Token::LOWEST_PRECEDENCE);
	ast::Expr* parse_primary_expression();
	ast::AST* get_ast()			{ return tree; }

	Token* parse_type();
	Token* parse_keyword();
	Token* parse_id();
};

inline std::unique_ptr<Syntax> g_syntax;