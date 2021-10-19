#pragma once

#include <ast/ast.h>

namespace semantic
{
	struct PrototypeInfo
	{
		std::unordered_map<std::string, ast::ExprDecl*> decls;

		ast::Prototype* pt = nullptr;

		PrototypeInfo& operator = (ast::Prototype* prototype)
		{
			decls.clear();
			pt = prototype;
			return *this;
		}
	};

	struct GlobalInfo
	{
		std::unordered_set<std::string> calls;
	};
}

class Semantic
{
private:

	std::vector<std::string> errors;

	semantic::GlobalInfo gi {};
	semantic::PrototypeInfo pi {};

	ast::AST* ast_tree = nullptr;
		
public:

	void print_errors();
	void add_variable(ast::ExprDecl* expr);

	bool run();
	bool analyze_prototype(ast::Prototype* prototype);
	bool analyze_body(ast::StmtBody* body);
	bool analyze_expr(ast::Expr* expr);
	bool analyze_if(ast::StmtIf* stmt_if);
	bool analyze_return(ast::StmtReturn* stmt_return);

	ast::ExprDecl* get_declared_variable(const std::string& name);
	ast::Prototype* get_prototype(const std::string& name);

	template <typename... A>
	inline bool add_error(const std::string& format, A... args)
	{
		errors.push_back(std::format(format, args...));
		return false;
	}
};

inline std::unique_ptr<Semantic> g_semantic;