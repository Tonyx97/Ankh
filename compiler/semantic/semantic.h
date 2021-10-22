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

		bool has_decl(ast::ExprDecl* decl)
		{
			return decls.find(decl->id->value) != decls.end();
		}
	};

	struct GlobalInfo
	{
		std::unordered_set<std::string> calls;
		std::unordered_map<std::string, ast::Prototype*> prototype_decls;
		std::unordered_map<std::string, ast::Prototype*> prototype_defs;

		void add_call(const std::string& name)				{ calls.insert(name); }
		void add_prototype_decl(ast::Prototype* prototype)	{ prototype_decls.insert({ prototype->id_token->value, prototype }); }
		void add_prototype_def(ast::Prototype* prototype)	{ prototype_defs.insert({ prototype->id_token->value, prototype }); }
	};
}

class Semantic
{
private:

	std::vector<std::string> errors;

	semantic::GlobalInfo g_ctx {};
	semantic::PrototypeInfo p_ctx {};

	ast::AST* ast = nullptr;
		
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
	ast::Prototype* get_declared_prototype(const std::string& name);
	ast::Prototype* get_defined_prototype(const std::string& name);

	template <typename... A>
	inline bool add_error(const std::string& format, A... args)
	{
		errors.push_back(std::format(format, args...));
		return false;
	}
};

inline std::unique_ptr<Semantic> g_semantic;