#pragma once

#include <lexer/lexer.h>

#include "types.h"

namespace ast
{
	/*
	* Base
	*/
	struct Base
	{
		StmtExprType stmt_type = Stmt_None;

		virtual ~Base() = default;
	};

	/*
	* Expr
	*/
	struct Expr : public Base
	{
		std::string name;

		Expr* lhs = nullptr,
			* rhs = nullptr;

		Type type {};

		Expr()								{ stmt_type = StmtExpr; }
		~Expr()								{}

		static bool check_class(Base* i)	{ return i->stmt_type > StmtExpr_Begin && i->stmt_type < StmtExpr_End; }
	};

	/*
	* ExprIntLiteral
	*/
	struct ExprIntLiteral : public Expr
	{
		ExprIntLiteral(const std::string& name, const Type& type)
		{
			stmt_type = StmtExpr_IntLiteral;
			this->name = name;
			this->type = type;
			this->type.lvalue = false;
		}

		static bool check_class(Base* i)	{ return i->stmt_type == StmtExpr_IntLiteral; }
	};

	/*
	* ExprStaticValue
	*/
	struct ExprStaticValue : public Expr
	{
		ExprStaticValue(const std::string& name, const Type& type)
		{
			stmt_type = StmtExpr_StaticValue;
			this->name = name;
			this->type = type;
		}

		static bool check_class(Base* i)	{ return i->stmt_type == StmtExpr_StaticValue; }
	};

	/*
	* ExprId
	*/
	struct ExprId : public Expr
	{
		ExprId(const std::string& name, const Type& type)
		{
			stmt_type = StmtExpr_Id;
			this->name = name;
			this->type = type;
		}
			
		static bool check_class(Base* i)	{ return i->stmt_type == StmtExpr_Id; }
	};

	/*
	* ExprDecl
	*/
	struct ExprDecl : public Expr
	{
		bool global = false;

		ExprDecl(const std::string& name, const Type& type, Expr* rhs = nullptr, bool global = false) : global(global)
		{
			stmt_type = StmtExpr_Decl;
			this->name = name;
			this->rhs = rhs;
			this->type = type;
		}

		~ExprDecl()							{ _FREE(rhs); }

		static bool check_class(Base* i)	{ return i->stmt_type == StmtExpr_Decl; }
	};

	/*
	* ExprAssign
	*/
	struct ExprAssign : public Expr
	{
		ExprAssign(const std::string& name, const Type& type, Expr* rhs)
		{
			stmt_type = StmtExpr_Assign;
			this->name = name;
			this->rhs = rhs;
			this->type = type;
		}

		~ExprAssign()						{ _FREE(rhs); }

		static bool check_class(Base* i)	{ return i->stmt_type == StmtExpr_Assign; }
	};

	/*
	* ExprBinaryOp
	*/
	struct ExprBinaryOp : public Expr
	{
		BinOpType op;

		ExprBinaryOp(Expr* lhs, Expr* rhs, BinOpType op, const Type& type) : op(op)
		{
			stmt_type = StmtExpr_BinOp;
			this->lhs = lhs;
			this->rhs = rhs;
			this->type = type;
		}

		~ExprBinaryOp()
		{
			_FREE(lhs);
			_FREE(rhs);
		}
			
		static bool check_class(Base* i)	{ return i->stmt_type == StmtExpr_BinOp; }
	};

	/*
	* UnaryOp
	*/
	struct ExprUnaryOp : public Expr
	{
		UnaryOpType op;

		bool on_left = false;

		ExprUnaryOp(Expr* rhs, UnaryOpType op, bool on_left = true) : op(op), on_left(on_left)
		{
			stmt_type = StmtExpr_UnaryOp;
			this->name = rhs->name;	// propagate the expected lvalue to the outer unary op
			this->rhs = rhs;
			this->type = rhs->type;
		}

		~ExprUnaryOp()						{ _FREE(rhs); }

		static bool check_class(Base* i)	{ return i->stmt_type == StmtExpr_UnaryOp; }
	};

	/*
	* ExprCall
	*/
	struct ExprCall : public Expr
	{
		std::vector<Expr*> stmts;

		struct Prototype* prototype = nullptr;

		bool intrinsic = false;

		ExprCall(Prototype* prototype, const std::string& name, const Type& type, bool built_in = false) : prototype(prototype), intrinsic(intrinsic)
		{
			stmt_type = StmtExpr_Call;
			this->name = name;
			this->type = type;
		}

		~ExprCall()
		{
			for (auto stmt : stmts)
				_FREE(stmt);
		}
			
		static bool check_class(Base* i)				{ return i->stmt_type == StmtExpr_Call; }
	};

	/*
	* ExprCast
	*/
	struct ExprCast : public Expr
	{
		bool implicit = false;

		ExprCast(Expr* rhs, const Type& type, bool implicit = true) : implicit(implicit)
											{ stmt_type = StmtExpr_Cast; this->rhs = rhs; this->type = type; }
		~ExprCast()							{ _FREE(rhs); }

		bool needs_ir_cast() const			{ return rhs->type.size != type.size; }

		static bool check_class(Base* i)	{ return i->stmt_type == StmtExpr_Cast; }
	};

	/*
	* StmtBody
	*/
	struct StmtBody : public Base
	{
		std::vector<Base*> stmts;

		StmtBody()						 { stmt_type = Stmt_Body; }
		~StmtBody()
		{
			for (auto stmt : stmts)
				_FREE(stmt);
		}

		static bool check_class(Base* i) { return i->stmt_type == Stmt_Body; }
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
											{ stmt_type = Stmt_If; }
		~StmtIf()
		{
			_FREE(else_body);

			for (auto _if : ifs)
				_FREE(_if);

			_FREE(if_body);
			_FREE(expr);
		}
			
		static bool check_class(Base* i)	{ return i->stmt_type == Stmt_If; }
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
											{ stmt_type = Stmt_For; }
		~StmtFor()
		{
			_FREE(body);
			_FREE(step);
			_FREE(init);
			_FREE(condition);
		}

		static bool check_class(Base* i) { return i->stmt_type == Stmt_For; }
	};

	/*
	* StmtWhile
	*/
	struct StmtWhile : public Base
	{
		Expr* condition = nullptr;

		StmtBody* body = nullptr;

		StmtWhile(Expr* condition, StmtBody* body) : condition(condition), body(body)
						{ stmt_type = Stmt_While; }
		~StmtWhile()
		{
			_FREE(body);
			_FREE(condition);
		}

		static bool check_class(Base* i) { return i->stmt_type == Stmt_While; }
	};

	/*
	* StmtDoWhile
	*/
	struct StmtDoWhile : public Base
	{
		Expr* condition = nullptr;

		StmtBody* body = nullptr;

		StmtDoWhile(Expr* condition, StmtBody* body) : condition(condition), body(body)
											{ stmt_type = Stmt_DoWhile; }
		~StmtDoWhile()
		{
			_FREE(body);
			_FREE(condition);
		}

		static bool check_class(Base* i)	{ return i->stmt_type == Stmt_DoWhile; }
	};

	/*
	* StmtBreak
	*/
	struct StmtBreak : public Base
	{
		StmtBreak()							{ stmt_type = Stmt_Break; }

		static bool check_class(Base* i)	{ return i->stmt_type == Stmt_Break; }
	};

	/*
	* StmtContinue
	*/
	struct StmtContinue : public Base
	{
		StmtContinue()						{ stmt_type = Stmt_Continue; }

		static bool check_class(Base* i)	{ return i->stmt_type == Stmt_Continue; }
	};

	/*
	* StmtReturn
	*/
	struct StmtReturn : public Base
	{
		Expr* expr = nullptr;
		
		Type ret_type {};

		StmtReturn(Expr* expr, const Type& ret_type) : expr(expr), ret_type(ret_type)
											{ stmt_type = Stmt_Return; }
		~StmtReturn()						{ _FREE(expr); }

		static bool check_class(Base* i)	{ return i->stmt_type == Stmt_Return; }
	};

	/*
	* Prototype
	*/
	struct Prototype
	{
		std::vector<Expr*> params;

		std::string name;

		Prototype* def = nullptr;

		StmtBody* body = nullptr;

		Type type {};

		Prototype(const std::string& name, const Type& type) : name(name), type(type) {}
		~Prototype()
		{
			for (auto param : params)
				_FREE(param);

			_FREE(body);
		}

		Expr* get_param(int i)
		{
			if (params.empty())
				return nullptr;

			return (i >= 0 && i < static_cast<int>(params.size()) ? params[i] : nullptr);
		}

		bool is_decl() const							{ return !body; }
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

		void print(AST* ast);
		void print_prototype(Prototype* prototype);
		void print_body(StmtBody* body);
		void print_stmt(Base* stmt);
		void print_if(StmtIf* stmt_if);
		void print_for(StmtFor* stmt_for);
		void print_while(StmtWhile* stmt_while);
		void print_do_while(StmtDoWhile* stmt_do_while);
		void print_break(StmtBreak* stmt_break);
		void print_continue(StmtContinue* stmt_continue);
		void print_return(StmtReturn* stmt_return);
		void print_expr(Expr* expr);
		void print_decl(ExprDecl* decl);
		void print_assign(ExprAssign* assign);
		void print_expr_int(ExprIntLiteral* expr);
		void print_static_val(ExprStaticValue* expr);
		void print_id(ExprId* expr);
		void print_expr_unary_op(ExprUnaryOp* expr);
		void print_expr_binary_op(ExprBinaryOp* expr);
		void print_expr_call(ExprCall* expr);
		void print_cast(ExprCast* expr);
	};
}