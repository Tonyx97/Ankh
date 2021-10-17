#pragma once

#include <lexer/lexer.h>

struct Int
{
	uint8_t size = 0;

	union
	{
		uint64_t u64;
		uint32_t u32;
		uint16_t u16;
		uint8_t u8;

		int64_t i64;
		int32_t i32;
		int16_t i16;
		int8_t i8;
	};

	template <typename T>
	static Int create(T val)
	{
		Int obj { 0 }; obj.u64 = val; obj.size = sizeof(T);
		return obj;
	}
};

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
	};

	/*
	* Expr
	*/
	struct Expr : public Base
	{
		Expr()								{ type = STMT_EXPR; }

		virtual void set_token(Token* ty) = 0;
		virtual Token* get_token() = 0;
		virtual std::string get_name() = 0;

		static bool check_class(Base* i)	{ return i->type >= STMT_EXPR && i->type <= EXPR_CALL; }
	};

	/*
	* ExprIntLiteral
	*/
	struct ExprIntLiteral : public Expr
	{
		Int value;
			
		Token* token = nullptr;

		ExprIntLiteral(Int value, Token ty) : value(value), token(token)
											{ type = EXPR_INT_LITERAL; }

		void set_token(Token* v)			{ token = v; }
		Token* get_token()					{ return token; }
		std::string get_name() override		{ return std::to_string(value.u64); };

		static bool check_class(Base* i)	{ return i->type == EXPR_INT_LITERAL; }
	};

	/*
	* ExprId
	*/
	struct ExprId : public Expr
	{
		std::string name;

		Token* token = nullptr;

		ExprId(const std::string& name) : name(name)
											{ type = EXPR_ID; }
			
		void set_token(Token* v)			{ token = v; }
		Token* get_token()					{ return token; }
		std::string get_name() override		{ return name; }

		static bool check_class(Base* i)	{ return i->type == EXPR_ID; }
	};

	/*
	* ExprDeclOrAssign
	*/
	struct ExprDeclOrAssign : public Expr
	{
		std::string name;

		Expr* value = nullptr;

		Token* token = nullptr;

		ExprDeclOrAssign(const std::string& name, Expr* value = nullptr, Token* token = nullptr) :
							name(name), value(value), token(token)
													{ type = EXPR_DECL_OR_ASSIGN; }

		bool is_declaration() const					{ return !!token; }
			
		void set_token(Token* v)					{ token = v; }
		Token* get_token()							{ return token; }
		std::string get_name() override				{ return name; }

		static bool check_class(Base* i)			{ return i->type == EXPR_DECL_OR_ASSIGN; }
	};

	/*
	* ExprBinaryOp
	*/
	struct ExprBinaryOp : public Expr
	{
		Expr* left = nullptr;

		Token* op_token = nullptr,
				* token = nullptr;

		Expr* right = nullptr;

		bool assign = false;

		ExprBinaryOp(Expr* left, Token op, Token ty, Expr* right, bool assign = false) :
						left(left), op_token(op_token), token(token), right(right), assign(assign)
											{ type = EXPR_BINARY_OP; }
			
		void set_token(Token* v)			{ token = v; }
		Token* get_token()					{ return token; }
		std::string get_name() override		{ return Lexer::STRIFY_TOKEN(op_token); };

		static bool check_class(Base* i)	{ return i->type == EXPR_BINARY_OP; }
	};

	/*
	* UnaryOp
	*/
	struct ExprUnaryOp : public Expr
	{
		Expr* value = nullptr;

		Token* op_token = nullptr;

		ExprUnaryOp(Token op, Expr* value) : op_token(op_token), value(value)
											{ type = EXPR_UNARY_OP; }

		void set_ty(Token* ty)			 {}
		Token* get_ty()					 { return nullptr; }
		std::string get_name() override  { return Lexer::STRIFY_TOKEN(op_token); };

		static bool check_class(Base* i) { return i->type == EXPR_UNARY_OP; }
	};

	/*
	* ExprCall
	*/
	struct ExprCall : public Expr
	{
		std::string name;

		std::vector<Expr*> stmts;

		Token* ret_token = nullptr;

		struct Prototype* prototype = nullptr;

		bool built_in = false;

		ExprCall(const std::string& name, bool built_in = false)
					: name(name), built_in(built_in)		{ type = EXPR_CALL; }
			
		void set_token(Token* v)						{ ret_token = v;}
		Token* get_token()								{ return ret_token; }
		std::string get_name() override					{ return name; }

		static bool check_class(Base* i)				{ return i->type == EXPR_CALL; }
	};

	/*
	* StmtBody
	*/
	struct StmtBody : public Base
	{
		std::vector<Base*> stmts;

		StmtBody()						 { type = STMT_BODY; }

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
			
		static bool check_class(Base* i) { return i->type == STMT_IF; }
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

		std::string name;

		StmtBody* body = nullptr;

		Token* ret_token = nullptr;

		Prototype(const std::string& name) : name(name)		{}

		bool is_declaration() const							{ return !body; }
	};

	struct AST
	{
		std::vector<Prototype*> prototypes;

		~AST()
		{
			// we need to free the whole ast tree lol
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