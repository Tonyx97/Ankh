#pragma once

#include <lexer/lexer.h>

namespace ast
{
	enum Type
	{
		STMT_NONE,
		STMT_EXPR,
			EXPR_BEGIN,
			EXPR_INT_LITERAL,
			EXPR_ID,
			EXPR_DECL,
			EXPR_ASSIGN,
			EXPR_BINARY_OP,
			EXPR_UNARY_OP,
			EXPR_CALL,
			EXPR_IMPLICIT_CAST,
			EXPR_END,
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
		Expr* lhs = nullptr,
			* rhs = nullptr;

		Expr()								{ type = STMT_EXPR; }
		~Expr()								{}

		virtual Token* get_token() = 0;
		virtual std::string get_name() = 0;

		static bool check_class(Base* i)	{ return i->type > EXPR_BEGIN && i->type < EXPR_END; }
	};

	/*
	* ExprIntLiteral
	*/
	struct ExprIntLiteral : public Expr
	{
		Token* token = nullptr;

		ExprIntLiteral(Token* token) : token(token)
											{ type = EXPR_INT_LITERAL; }

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
			
		Token* get_token()					{ return token; }
		std::string get_name()				{ return token->value; }

		static bool check_class(Base* i)	{ return i->type == EXPR_ID; }
	};

	/*
	* ExprDecl
	*/
	struct ExprDecl : public Expr
	{
		Token* id_token = nullptr,
			 * type_token = nullptr;

		bool global = false;

		ExprDecl(Token* id_token, Token* type_token = nullptr, Expr* rhs = nullptr, bool global = false) :
				id_token(id_token), type_token(type_token), global(global)
											{ type = EXPR_DECL; this->rhs = rhs; }
		~ExprDecl()							{ _FREE(rhs); }

		Token* get_token()					{ return type_token; }
		std::string get_name()				{ return id_token->value; }

		static bool check_class(Base* i)	{ return i->type == EXPR_DECL; }
	};

	/*
	* ExprAssign
	*/
	struct ExprAssign : public Expr
	{
		Token* id_token = nullptr;

		ExprAssign(Token* id_token, Expr* expr) : id_token(id_token)
											{ type = EXPR_ASSIGN; this->rhs = rhs; }
		~ExprAssign()						{ _FREE(rhs); }

		Token* get_token()					{ return id_token; }
		std::string get_name()				{ return id_token->value; }

		static bool check_class(Base* i)	{ return i->type == EXPR_ASSIGN; }
	};

	/*
	* ExprBinaryOp
	*/
	struct ExprBinaryOp : public Expr
	{
		Token* token = nullptr,
			 * type_token = nullptr;

		ExprBinaryOp(Expr* lhs, Expr* rhs, Token* token) : token(token)
		{
			type = EXPR_BINARY_OP;
			this->lhs = lhs;
			this->rhs = rhs;
		}

		~ExprBinaryOp()
		{
			_FREE(lhs);
			_FREE(rhs);
		}
			
		Token* get_token()					{ return token; }
		std::string get_name()				{ return Lexer::STRIFY_TOKEN(token); };

		static bool check_class(Base* i)	{ return i->type == EXPR_BINARY_OP; }
	};

	/*
	* UnaryOp
	*/
	struct ExprUnaryOp : public Expr
	{
		Token* token = nullptr;

		ExprUnaryOp(Expr* rhs, Token* token) : token(token)
											{ type = EXPR_UNARY_OP; this->rhs = rhs; }
		~ExprUnaryOp()						{ _FREE(rhs); }

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

		ExprCall(Token* id_token, Token* ret_token, bool built_in = false) : id_token(id_token), ret_token(ret_token), built_in(built_in)
														{ type = EXPR_CALL; }
		~ExprCall()
		{
			for (auto stmt : stmts)
				_FREE(stmt);
		}
			
		Token* get_token()								{ return ret_token; }
		std::string get_name()							{ return id_token->value; }

		static bool check_class(Base* i)				{ return i->type == EXPR_CALL; }
	};

	/*
	* ExprCast
	*/
	struct ExprCast : public Expr
	{
		TokenID cast_type = Token_None;

		bool implicit = false;

		ExprCast(Expr* rhs, TokenID cast_type, bool implicit = true) : cast_type(cast_type), implicit(implicit)
											{ type = EXPR_IMPLICIT_CAST; this->rhs = rhs; }
		~ExprCast()							{ _FREE(rhs); }

		Token* get_token()					{ return nullptr; }
		std::string get_name()				{ return {}; }

		static bool check_class(Base* i)	{ return i->type == EXPR_IMPLICIT_CAST; }
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
		~StmtIf()
		{
			_FREE(else_body);

			for (auto _if : ifs)
				_FREE(_if);

			_FREE(if_body);
			_FREE(expr);
		}
			
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
		~StmtFor()
		{
			_FREE(body);
			_FREE(step);
			_FREE(init);
			_FREE(condition);
		}

		static bool check_class(Base* i) { return i->type == STMT_FOR; }
	};

	/*
	* StmtReturn
	*/
	struct StmtReturn : public Base
	{
		Expr* expr = nullptr;
		
		Token* ret_token = nullptr;

		StmtReturn(Expr* expr, Token* ret_token) : expr(expr), ret_token(ret_token)
											{ type = STMT_RETURN; }
		~StmtReturn()						{ _FREE(expr); }

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
	};

	struct AST
	{
		std::vector<ExprDecl*> global_decls;
		std::vector<Prototype*> prototypes;

		~AST()
		{
			for (auto prototype : prototypes)	_FREE(prototype);
			for (auto decl : global_decls)		_FREE(decl);
		}
	};

	struct Printer
	{
		int curr_level = 0;

		bool first_prototype_printed = false;

		void print(AST* tree);
		void print_prototype(Prototype* prototype);
		void print_body(StmtBody* body);
		void print_stmt(Base* stmt);
		void print_if(StmtIf* stmt_if);
		void print_for(StmtFor* stmt_for);
		void print_return(StmtReturn* stmt_return);
		void print_expr(Expr* expr);
		void print_decl(ExprDecl* decl);
		void print_assign(ExprAssign* assign);
		void print_expr_int(ExprIntLiteral* expr);
		void print_id(ExprId* expr);
		void print_expr_unary_op(ExprUnaryOp* expr);
		void print_expr_binary_op(ExprBinaryOp* expr);
		void print_expr_call(ExprCall* expr);
		void print_cast(ExprCast* expr);
	};
}