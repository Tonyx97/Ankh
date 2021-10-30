#pragma once

#include <ast/ast.h>

namespace syntax
{
	struct PrototypeContext
	{
		ast::Prototype* fn = nullptr;

		PrototypeContext& operator = (ast::Prototype* prototype)
		{
			fn = prototype;
			return *this;
		}
	};

	struct GlobalContext
	{
		std::unordered_map<std::string, ast::Prototype*> prototypes;

		bool expect_semicolon = false;

		void add_prototype(ast::Prototype* prototype) { prototypes.insert({ prototype->name, prototype }); }

		ast::Prototype* get_prototype(const std::string& name)
		{
			auto it = prototypes.find(name);
			return it != prototypes.end() ? it->second : nullptr;
		}
	};
}

class Syntax
{
private:

	syntax::PrototypeContext p_ctx {};
	syntax::GlobalContext g_ctx {};

	ast::AST* ast = nullptr;

public:

	Syntax();
	~Syntax();

	void print_ast();
	void run();

	template <typename T>
	T* return_and_expect_semicolon(T* v)
	{
		g_ctx.expect_semicolon = true;
		return v;
	}

	ast::Prototype* parse_prototype();
	ast::StmtBody* parse_body(ast::StmtBody* body);
	ast::Base* parse_statement();
	ast::Expr* parse_expression();
	ast::Expr* parse_expression_precedence(ast::Expr* lhs, int min_precedence = Token::LOWEST_PRECEDENCE);
	ast::Expr* parse_primary_expression();
	ast::AST* get_ast()			{ return ast; }

	ast::TypeOpt parse_type(bool expect = false);

	std::vector<ast::Expr*> parse_prototype_params_decl();
	std::vector<ast::Expr*> parse_call_params(ast::Prototype* prototype);
};

inline std::unique_ptr<Syntax> g_syntax;