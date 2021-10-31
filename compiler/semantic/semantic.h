#pragma once

#include <ast/ast.h>

namespace semantic
{
	struct PrototypeInfo
	{
		std::unordered_map<std::string, ast::Type> id_types;
		std::unordered_set<ast::Expr*> analyzed_exprs;

		ast::Prototype* pt = nullptr;

		void set_id_type(const std::string& id, const ast::Type& type);

		ast::TypeOpt get_id_type(const std::string& id);

		PrototypeInfo& operator = (ast::Prototype* prototype)
		{
			id_types.clear();
			analyzed_exprs.clear();
			pt = prototype;
			return *this;
		}
	};

	struct GlobalInfo
	{
		std::unordered_map<std::string, ast::Prototype*> prototypes;

		void add_prototype(ast::Prototype* prototype) { prototypes.insert({ prototype->name, prototype }); }
		
		ast::Prototype* get_prototype(const std::string& name)
		{
			const auto it = prototypes.find(name);
			return (it == prototypes.end() ? nullptr : it->second);
		}
	};

	struct BodyData 
	{
		int depth = 0;

		bool is_in_loop = false;
	};
}

class Semantic
{
private:

	std::vector<std::string> errors;

	semantic::GlobalInfo g_ctx {};
	semantic::PrototypeInfo p_ctx {};

	ast::AST* ast = nullptr;

	void enter_scope();
	void exit_scope();
	
	void analyze_function(ast::Prototype* function);

	void analyze_body(ast::StmtBody* body, semantic::BodyData data);

	void analyze_if(ast::StmtIf* stmt, semantic::BodyData data);
	void analyze_for(ast::StmtFor* stmt, semantic::BodyData data);
	void analyze_while(ast::StmtWhile* stmt, semantic::BodyData data);
	void analyze_do_while(ast::StmtDoWhile* stmt, semantic::BodyData data);
	void analyze_return(ast::StmtReturn* stmt, semantic::BodyData data);

	void analyze_expr(ast::Expr* expr);
	void analyze_expr_id(ast::ExprId* expr);
	void analyze_expr_decl(ast::ExprDecl* expr);
	void analyze_expr_assign(ast::ExprAssign* expr);
	void analyze_expr_bin_assign(ast::ExprBinaryAssign* expr);
	void analyze_expr_bin_op(ast::ExprBinaryOp* expr);
	void analyze_expr_unary_op(ast::ExprUnaryOp* expr);
	void analyze_expr_call(ast::ExprCall* expr);
	void analyze_expr_cast(ast::ExprCast* expr);

	ast::Type get_expr_type(ast::Expr* expr);
	ast::Type get_id_type(const std::string& id);

	ast::Expr* implicit_cast(ast::Expr* expr, const ast::Type& type);
	void implicit_cast_replace(ast::Expr*& expr, const ast::Type& type);

public:

	void print_errors();

	bool run();

	template <typename... A>
	inline bool add_error(const std::string& format, A... args)
	{
		errors.push_back(std::format(format, args...));
		return false;
	}
};

inline std::unique_ptr<Semantic> g_semantic;