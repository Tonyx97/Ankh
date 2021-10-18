#pragma once

#include <lexer/lexer.h>

namespace ast
{
	enum Type
	{
		STMT_NONE,
		STMT_EXPR,
			EXPR_INT_LITERAL,
			EXPR_ID,
			EXPR_DECL_OR_ASSIGN,
			EXPR_BINARY_OP,
			EXPR_UNARY_OP,
			EXPR_CALL,
		STMT_BODY,
		STMT_IF,
		STMT_FOR,
		STMT_RETURN,
	};

	/*
	* Base
	*/
	struct Base
	{
		Type type = STMT_NONE;

		virtual ~Base() = default;
	};

	/*
	* Expr
	*/
	struct Expr : public Base
	{
		Expr()								{ type = STMT_EXPR; }

		virtual void set_token(Token* v) = 0;
		virtual Token* get_token() = 0;
		virtual std::string get_name() = 0;

		static bool check_class(Base* i)	{ return i->type >= STMT_EXPR && i->type <= EXPR_CALL; }
	};

	/*
	* ExprIntLiteral
	*/
	struct ExprIntLiteral : public Expr
	{
		Token* token = nullptr;

		ExprIntLiteral(Token* token) : token(token)
											{ type = EXPR_INT_LITERAL; }

		void set_token(Token* v)			{ token = v; }
		Token* get_token()					{ return token; }
		std::string get_name()				{ return token->value; };

		static bool check_class(Base* i)	{ return i->type == EXPR_INT_LITERAL; }
	};

	/*
	* ExprId
	*/
	struct ExprId : public Expr
	{
		Token* token = nullptr;

		ExprId(Token* token) : token(token) { type = EXPR_ID; }
			
		void set_token(Token* v)			{ token = v; }
		Token* get_token()					{ return token; }
		std::string get_name()				{ return token->value; }

		static bool check_class(Base* i)	{ return i->type == EXPR_ID; }
	};

	/*
	* ExprDeclOrAssign
	*/
	struct ExprDeclOrAssign : public Expr
	{
		Expr* value = nullptr;

		Token* id_token = nullptr,
			 * type_token = nullptr;

		ExprDeclOrAssign(Token* id_token, Token* type_token = nullptr, Expr* value = nullptr) :
							id_token(id_token), type_token(type_token), value(value)
													{ type = EXPR_DECL_OR_ASSIGN; }
		~ExprDeclOrAssign()
		{
			_FREE(value);
		}

		bool is_declaration() const					{ return !!type_token; }
			
		void set_token(Token* v)					{ type_token = v; }
		Token* get_token()							{ return type_token; }
		std::string get_name()						{ return id_token->value; }

		static bool check_class(Base* i)			{ return i->type == EXPR_DECL_OR_ASSIGN; }
	};

	/*
	* ExprBinaryOp
	*/
	struct ExprBinaryOp : public Expr
	{
		Expr* left = nullptr,
			* right = nullptr;

		Token* token = nullptr;

		ExprBinaryOp(Expr* left, Expr* right, Token* token) :
						left(left), right(right), token(token)
											{ type = EXPR_BINARY_OP; }
		~ExprBinaryOp()
		{
			_FREE(left);
			_FREE(right);
		}
			
		void set_token(Token* v)			{ token = v; }
		Token* get_token()					{ return token; }
		std::string get_name()				{ return Lexer::STRIFY_TOKEN(token); };

		static bool check_class(Base* i)	{ return i->type == EXPR_BINARY_OP; }
	};

	/*
	* UnaryOp
	*/
	struct ExprUnaryOp : public Expr
	{
		Expr* value = nullptr;

		Token* token = nullptr;

		ExprUnaryOp(Expr* value, Token* token) : value(value), token(token)
											{ type = EXPR_UNARY_OP; }

		void set_token(Token* v)			{ token = v; }
		Token* get_token()					{ return token; }
		std::string get_name()				{ return Lexer::STRIFY_TOKEN(token); };

		static bool check_class(Base* i)	{ return i->type == EXPR_UNARY_OP; }
	};

	/*
	* ExprCall
	*/
	struct ExprCall : public Expr
	{
		std::vector<Expr*> stmts;

		Token* id_token = nullptr,
			 * ret_token = nullptr;

		struct Prototype* prototype = nullptr;

		bool built_in = false;

		ExprCall(Token* id_token, bool built_in = false) : id_token(id_token), built_in(built_in)
														{ type = EXPR_CALL; }
		~ExprCall()
		{
			for (auto stmt : stmts)
				_FREE(stmt);
		}
			
		void set_token(Token* v)						{ ret_token = v;}
		Token* get_token()								{ return ret_token; }
		std::string get_name() override					{ return id_token->value; }

		static bool check_class(Base* i)				{ return i->type == EXPR_CALL; }
	};

	/*
	* StmtBody
	*/
	struct StmtBody : public Base
	{
		std::vector<Base*> stmts;

		StmtBody()						 { type = STMT_BODY; }
		~StmtBody()
		{
			for (auto stmt : stmts)
				_FREE(stmt);
		}

		static bool check_class(Base* i) { return i->type == STMT_BODY; }
	};

	/*
	* StmtIf
	*/
	struct StmtIf : public Base
	{
		Expr* expr = nullptr;

		std::vector<StmtIf*> ifs;

		StmtBody* if_body = nullptr,
				* else_body = nullptr;

		StmtIf(Expr* expr, StmtBody* if_body) : expr(expr), if_body(if_body) 
											{ type = STMT_IF; }
			
		static bool check_class(Base* i)	{ return i->type == STMT_IF; }
	};

	/*
	* StmtFor
	*/
	struct StmtFor : public Base
	{
		Expr* condition = nullptr;

		Base* init = nullptr,
			* step = nullptr;

		StmtBody* body = nullptr;

		StmtFor(Expr* condition, Base* init, Base* step, StmtBody* body)
				: condition(condition), init(init), step(step), body(body)
											{ type = STMT_FOR; }

		static bool check_class(Base* i) { return i->type == STMT_FOR; }
	};

	/*
	* StmtReturn
	*/
	struct StmtReturn : public Base
	{
		Expr* expr = nullptr;

		StmtReturn(Expr* expr) : expr(expr) { type = STMT_RETURN; }

		static bool check_class(Base* i)	{ return i->type == STMT_RETURN; }
	};

	/*
	* Prototype
	*/
	struct Prototype
	{
		std::vector<Base*> params;

		StmtBody* body = nullptr;

		Token* id_token = nullptr,
			 * ret_token = nullptr;

		Prototype(Token* id_token) : id_token(id_token)		{}
		~Prototype()
		{
			for (auto param : params)
				_FREE(param);

			_FREE(body);
		}

		bool is_declaration() const							{ return !body; }
	};

	struct AST
	{
		std::vector<Prototype*> prototypes;

		~AST()
		{
			for (auto prototype : prototypes)
				_FREE(prototype);
		}
	};

	struct Printer
	{
		int curr_level = 0;

		bool first_prototype_printed = false;

		void print(AST* tree);
		void print_prototype(Prototype* prototype);
		void print_body(ast::StmtBody* body);
		void print_stmt(ast::Base* stmt);
		void print_if(ast::StmtIf* stmt_if);
		void print_for(ast::StmtFor* stmt_for);
		void print_return(ast::StmtReturn* stmt_return);
		void print_expr(ast::Expr* expr);
		void print_decl_or_assign(ast::ExprDeclOrAssign* assign);
		void print_expr_int(ast::ExprIntLiteral* expr);
		void print_id(ast::ExprId* expr);
		void print_expr_unary_op(ast::ExprUnaryOp* expr);
		void print_expr_binary_op(ast::ExprBinaryOp* expr);
		void print_expr_call(ast::ExprCall* expr);
	};
}