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
		STMT_WHILE,
		STMT_RETURN,
	};

	/*
	* Base
	*/
	struct Base
	{
		Type base_type = STMT_NONE;

		virtual ~Base() = default;
	};

	/*
	* Expr
	*/
	struct Expr : public Base
	{
		Expr* lhs = nullptr,
			* rhs = nullptr;

		Token* id = nullptr,
			 * type = nullptr;

		Expr()								{ base_type = STMT_EXPR; }
		~Expr()								{}

		virtual Token* get_token() = 0;
		virtual std::string get_name() = 0;

		static bool check_class(Base* i)	{ return i->base_type > EXPR_BEGIN && i->base_type < EXPR_END; }
	};

	/*
	* ExprIntLiteral
	*/
	struct ExprIntLiteral : public Expr
	{
		ExprIntLiteral(Token* type)
		{
			base_type = EXPR_INT_LITERAL;
			this->type = type;
		}

		Token* get_token()					{ return type; }
		std::string get_name()				{ return type->value; };

		static bool check_class(Base* i)	{ return i->base_type == EXPR_INT_LITERAL; }
	};

	/*
	* ExprId
	*/
	struct ExprId : public Expr
	{
		ExprId(Token* type)					{ base_type = EXPR_ID; this->id = type; }
			
		Token* get_token()					{ return id; }
		std::string get_name()				{ return id->value; }

		static bool check_class(Base* i)	{ return i->base_type == EXPR_ID; }
	};

	/*
	* ExprDecl
	*/
	struct ExprDecl : public Expr
	{
		bool global = false;

		ExprDecl(Token* id, Token* type = nullptr, Expr* rhs = nullptr, bool global = false) : global(global)
											{ base_type = EXPR_DECL; this->rhs = rhs; this->id = id; this->type = type; }
		~ExprDecl()							{ _FREE(rhs); }

		Token* get_token()					{ return type; }
		std::string get_name()				{ return id->value; }

		static bool check_class(Base* i)	{ return i->base_type == EXPR_DECL; }
	};

	/*
	* ExprAssign
	*/
	struct ExprAssign : public Expr
	{
		ExprAssign(Token* id, Expr* rhs)	{ base_type = EXPR_ASSIGN; this->rhs = rhs; this->id = id; }
		~ExprAssign()						{ _FREE(rhs); }

		Token* get_token()					{ return id; }
		std::string get_name()				{ return id->value; }

		static bool check_class(Base* i)	{ return i->base_type == EXPR_ASSIGN; }
	};

	/*
	* ExprBinaryOp
	*/
	struct ExprBinaryOp : public Expr
	{
		ExprBinaryOp(Expr* lhs, Expr* rhs, Token* op_id, Token* type)
		{
			base_type = EXPR_BINARY_OP;
			this->lhs = lhs;
			this->rhs = rhs;
			this->id = op_id;
			this->type = type;
		}

		~ExprBinaryOp()
		{
			_FREE(lhs);
			_FREE(rhs);
		}
			
		Token* get_token()					{ return id; }
		std::string get_name()				{ return Lexer::STRIFY_TOKEN(id); };

		static bool check_class(Base* i)	{ return i->base_type == EXPR_BINARY_OP; }
	};

	/*
	* UnaryOp
	*/
	struct ExprUnaryOp : public Expr
	{
		ExprUnaryOp(Expr* rhs, Token* type) { base_type = EXPR_UNARY_OP; this->rhs = rhs; this->type = type; }
		~ExprUnaryOp()						{ _FREE(rhs); }

		Token* get_token()					{ return type; }
		std::string get_name()				{ return Lexer::STRIFY_TOKEN(type); };

		static bool check_class(Base* i)	{ return i->base_type == EXPR_UNARY_OP; }
	};

	/*
	* ExprCall
	*/
	struct ExprCall : public Expr
	{
		std::vector<Expr*> stmts;

		struct Prototype* prototype = nullptr;

		bool built_in = false;

		ExprCall(Token* id_token, Token* type, bool built_in = false) : built_in(built_in)
														{ base_type = EXPR_CALL; this->id = id; this->type = type; }
		~ExprCall()
		{
			for (auto stmt : stmts)
				_FREE(stmt);
		}
			
		Token* get_token()								{ return type; }
		std::string get_name()							{ return id->value; }

		static bool check_class(Base* i)				{ return i->base_type == EXPR_CALL; }
	};

	/*
	* ExprCast
	*/
	struct ExprCast : public Expr
	{
		TokenID cast_type = Token_None;

		bool implicit = false;

		ExprCast(Expr* rhs, TokenID cast_type, bool implicit = true) : cast_type(cast_type), implicit(implicit)
											{ base_type = EXPR_IMPLICIT_CAST; this->rhs = rhs; }
		~ExprCast()							{ _FREE(rhs); }

		Token* get_token()					{ return nullptr; }
		std::string get_name()				{ return {}; }

		static bool check_class(Base* i)	{ return i->base_type == EXPR_IMPLICIT_CAST; }
	};

	/*
	* StmtBody
	*/
	struct StmtBody : public Base
	{
		std::vector<Base*> stmts;

		StmtBody()						 { base_type = STMT_BODY; }
		~StmtBody()
		{
			for (auto stmt : stmts)
				_FREE(stmt);
		}

		static bool check_class(Base* i) { return i->base_type == STMT_BODY; }
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
											{ base_type = STMT_IF; }
		~StmtIf()
		{
			_FREE(else_body);

			for (auto _if : ifs)
				_FREE(_if);

			_FREE(if_body);
			_FREE(expr);
		}
			
		static bool check_class(Base* i)	{ return i->base_type == STMT_IF; }
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
											{ base_type = STMT_FOR; }
		~StmtFor()
		{
			_FREE(body);
			_FREE(step);
			_FREE(init);
			_FREE(condition);
		}

		static bool check_class(Base* i) { return i->base_type == STMT_FOR; }
	};

	/*
	* StmtFor
	*/
	struct StmtWhile : public Base
	{
		Expr* condition = nullptr;

		StmtBody* body = nullptr;

		StmtWhile(Expr* condition, StmtBody* body) : condition(condition), body(body)
						{ base_type = STMT_WHILE; }
		~StmtWhile()
		{
			_FREE(body);
			_FREE(condition);
		}

		static bool check_class(Base* i) { return i->base_type == STMT_WHILE; }
	};

	/*
	* StmtReturn
	*/
	struct StmtReturn : public Base
	{
		Expr* expr = nullptr;
		
		Token* ret_token = nullptr;

		StmtReturn(Expr* expr, Token* ret_token) : expr(expr), ret_token(ret_token)
											{ base_type = STMT_RETURN; }
		~StmtReturn()						{ _FREE(expr); }

		static bool check_class(Base* i)	{ return i->base_type == STMT_RETURN; }
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
		void print_while(StmtWhile* stmt_while);
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