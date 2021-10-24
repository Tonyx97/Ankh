#pragma once

#include <ir/types.h>
#include <ast/types.h>

enum TokenID : int
{
	Token_None = 0,
	Token_Eof,

	Token_Id,
	Token_Keyword,
	Token_IntLiteral,
	Token_Comment,

	Token_AddAssign,
	Token_Add,
	Token_SubAssign,
	Token_Sub,
	Token_MulAssign,
	Token_Mul,
	Token_Indirection = Token_Mul,
	Token_DivAssign,
	Token_Div,
	Token_ModAssign,
	Token_Mod,
	Token_AndAssign,
	Token_And,
	Token_OrAssign,
	Token_Or,
	Token_XorAssign,
	Token_Xor,
	Token_ShrAssign,
	Token_Shr,
	Token_ShlAssign,
	Token_Shl,
	Token_Equal,
	Token_NotEqual,
	Token_Lt,
	Token_Lte,
	Token_Gt,
	Token_Gte,
	Token_LogicalAnd,
	Token_LogicalOr,

	Token_Colon,
	Token_Semicolon,
	Token_Comma,

	Token_ParenOpen,
	Token_ParenClose,
	Token_BraceOpen,
	Token_BraceClose,
	Token_BracketOpen,
	Token_BracketClose,
	Token_Not,
	Token_Assign,

	Token_Inc,
	Token_Dec,
	Token_LogicalNot,

	Token_Void,
	Token_Bool,
	Token_U8,
	Token_U16,
	Token_U32,
	Token_U64,
	Token_I8,
	Token_I16,
	Token_I32,
	Token_I64,
	Token_M128,

	Token_True,
	Token_False,

	Token_For,
	Token_While,
	Token_Do,
	Token_If,
	Token_Else,
	Token_Break,
	Token_Continue,
	Token_Return,
	Token_Extern,
};

struct Token
{
	static constexpr int LOWEST_PRECEDENCE = 16;

	std::string value = "";

	Int integer;

	TokenID id = Token_None;

	uint64_t flags = TypeFlag_None;

	int precedence = LOWEST_PRECEDENCE,
		line = 0;

	bool valid = false;
	
	uint8_t size = 0;

	ast::Type to_ast_type(int indirection = 0)
	{
		ast::Type v {};

		switch (id)
		{
		case Token_Void:	v.type = Type_Void; break;
		case Token_Bool:
		case Token_U8:
		case Token_I8:		v.type = Type_i8;   break;
		case Token_U16:
		case Token_I16:		v.type = Type_i16;  break;
		case Token_U32:
		case Token_I32:		v.type = Type_i32;  break;
		case Token_U64:
		case Token_I64:		v.type = Type_i64;  break;
		}

		v.indirection = indirection;
		v.integer = integer;
		v.size = size;
		v.flags = flags;

		return v;
	}

	BinOpType to_bin_op_type()
	{
		// make this a switch pls, prone to fail when refactoring...

		return static_cast<BinOpType>(static_cast<int>(id) - static_cast<int>(Token_AddAssign));
	}

	UnaryOpType to_unary_op_type()
	{
		switch (id)
		{
		case Token_Add:			return UnaryOpType_Add;
		case Token_Sub:			return UnaryOpType_Sub;
		case Token_Mul:			return UnaryOpType_Mul;
		case Token_And:			return UnaryOpType_And;
		case Token_Not:			return UnaryOpType_Not;
		case Token_Inc:			return UnaryOpType_Inc;
		case Token_Dec:			return UnaryOpType_Dec;
		case Token_LogicalNot:	return UnaryOpType_LogicalNot;
		case Token_LogicalAnd:	return UnaryOpType_LogicalAnd;
		}

		return UnaryOpType_None;
	}

	Token& operator = (const Token& token)
	{
		if (this != std::addressof(token))
		{
			value = token.value;
			integer = token.integer;
			id = token.id;
			flags = token.flags;
			precedence = token.precedence;
			line = token.line;
			valid = true;
		}

		return *this;
	}
};

inline std::unordered_map<std::string, TokenID> g_keywords =
{
	{ "for",		Token_For },
	{ "while",		Token_While },
	{ "do",			Token_Do },
	{ "if",			Token_If },
	{ "else",		Token_Else },
	{ "break",		Token_Break },
	{ "continue",	Token_Continue },
	{ "return",		Token_Return },
	{ "extern",		Token_Extern },		// not an statement but we put it here for now
};

inline std::unordered_map<std::string, std::tuple<TokenID, uint8_t, TypeFlag>> g_keywords_type =
{
	{ "void",	{ Token_Void,	 0,  TypeFlag_None } },
	{ "bool",	{ Token_Bool,	 8,  TypeFlag_Unsigned } },
	{ "u8",		{ Token_U8,		 8,  TypeFlag_Unsigned } },
	{ "u16",	{ Token_U16,	16,  TypeFlag_Unsigned } },
	{ "u32",	{ Token_U32,	32,  TypeFlag_Unsigned } },
	{ "u64",	{ Token_U64,	64,  TypeFlag_Unsigned } },
	{ "i8",		{ Token_I8,		 8,  TypeFlag_None } },
	{ "i16",	{ Token_I16,	16,  TypeFlag_None } },
	{ "i32",	{ Token_I32,	32,  TypeFlag_None } },
	{ "i64",	{ Token_I64,	64,  TypeFlag_None } },
	{ "m128",	{ Token_M128,	128, TypeFlag_None } },
};

inline std::unordered_map<std::string, std::tuple<TokenID, uint8_t>> g_static_values =
{
	{ "true",	{ Token_True,	8 } },
	{ "false",	{ Token_False,	8 } },
};

inline Token g_static_tokens[] =
{
	{ .value = ">>=", .id = Token_ShrAssign, .flags = TypeFlag_Op | TypeFlag_Assignation, .precedence = 14 },
	{ .value = "<<=", .id = Token_ShlAssign, .flags = TypeFlag_Op | TypeFlag_Assignation, .precedence = 14 },

	{ .value = "==", .id = Token_Equal,			.flags = TypeFlag_Op,							.precedence = 7 },
	{ .value = "!=", .id = Token_NotEqual,		.flags = TypeFlag_Op,							.precedence = 7 },
	{ .value = ">=", .id = Token_Gte,			.flags = TypeFlag_Op,							.precedence = 6 },
	{ .value = "<=", .id = Token_Lte,			.flags = TypeFlag_Op,							.precedence = 6 },
	{ .value = "+=", .id = Token_AddAssign,		.flags = TypeFlag_Op | TypeFlag_Assignation,	.precedence = 14 },
	{ .value = "-=", .id = Token_SubAssign,		.flags = TypeFlag_Op | TypeFlag_Assignation,	.precedence = 14 },
	{ .value = "++", .id = Token_Inc,			.flags = TypeFlag_Op,							.precedence = 1 },
	{ .value = "--", .id = Token_Dec,			.flags = TypeFlag_Op,							.precedence = 1 },
	{ .value = "*=", .id = Token_MulAssign,		.flags = TypeFlag_Op | TypeFlag_Assignation,	.precedence = 14 },
	{ .value = "%=", .id = Token_ModAssign,		.flags = TypeFlag_Op | TypeFlag_Assignation,	.precedence = 14 },
	{ .value = "/=", .id = Token_DivAssign,		.flags = TypeFlag_Op | TypeFlag_Assignation,	.precedence = 14 },
	{ .value = "&=", .id = Token_AndAssign,		.flags = TypeFlag_Op | TypeFlag_Assignation,	.precedence = 14 },
	{ .value = "|=", .id = Token_OrAssign,		.flags = TypeFlag_Op | TypeFlag_Assignation,	.precedence = 14 },
	{ .value = "^=", .id = Token_XorAssign,		.flags = TypeFlag_Op | TypeFlag_Assignation,	.precedence = 14 },
	{ .value = "&&", .id = Token_LogicalAnd,	.flags = TypeFlag_Op,							.precedence = 11 },
	{ .value = "||", .id = Token_LogicalOr,		.flags = TypeFlag_Op,							.precedence = 12 },
	{ .value = ">>", .id = Token_Shr,			.flags = TypeFlag_Op,							.precedence = 5 },
	{ .value = "<<", .id = Token_Shl,			.flags = TypeFlag_Op,							.precedence = 5 },

	{ .value = ";", .id = Token_Semicolon },
	{ .value = ",", .id = Token_Comma,		.flags = TypeFlag_None,						.precedence = 15 },
	{ .value = "(", .id = Token_ParenOpen,	.flags = TypeFlag_None,						.precedence = 1 },
	{ .value = ")", .id = Token_ParenClose,	.flags = TypeFlag_None,						.precedence = 1 },
	{ .value = "{", .id = Token_BracketOpen },
	{ .value = "}", .id = Token_BracketClose },
	{ .value = "[", .id = Token_BraceOpen,	.flags = TypeFlag_Op,							.precedence = 1 },
	{ .value = "]", .id = Token_BraceClose,	.flags = TypeFlag_Op,							.precedence = 1 },
	{ .value = "+", .id = Token_Add,		.flags = TypeFlag_Op,							.precedence = 4 },
	{ .value = "-", .id = Token_Sub,		.flags = TypeFlag_Op,							.precedence = 4 },
	{ .value = "*", .id = Token_Mul,		.flags = TypeFlag_Op,							.precedence = 3 },
	{ .value = "%", .id = Token_Mod,		.flags = TypeFlag_Op,							.precedence = 3 },
	{ .value = "/", .id = Token_Div,		.flags = TypeFlag_Op,							.precedence = 3 },
	{ .value = "&", .id = Token_And,		.flags = TypeFlag_Op,							.precedence = 8 },
	{ .value = "|", .id = Token_Or,			.flags = TypeFlag_Op,							.precedence = 10 },
	{ .value = "^", .id = Token_Xor,		.flags = TypeFlag_Op,							.precedence = 9 },
	{ .value = "~", .id = Token_Not,		.flags = TypeFlag_Op,							.precedence = 2 },
	{ .value = "!", .id = Token_LogicalNot,	.flags = TypeFlag_Op,							.precedence = 2 },
	{ .value = "=", .id = Token_Assign,		.flags = TypeFlag_Op | TypeFlag_Assignation,	.precedence = 14 },
	{ .value = ">", .id = Token_Gt,			.flags = TypeFlag_Op,							.precedence = 6 },
	{ .value = "<", .id = Token_Lt,			.flags = TypeFlag_Op,							.precedence = 6 },
};

namespace regex
{
	static inline std::regex SINGLE_COMMENT("\\/{2}.*$"),
							 WORD("\\b[a-zA-Z_][a-zA-Z0-9_]*\\b"),
							 INT_LITERAL("([0-9]{1,20})((u|i)(8|16|32|64))?");
}

class Lexer
{
private:

	std::vector<Token*> tokens,
						eaten_tokens;

	std::vector<std::string> errors;

public:

	~Lexer();

	bool run(const std::string& filename);

	void print_list();
	void print_errors();

	template <typename... A>
	inline void add_error(const std::string& format, A... args)
	{
		errors.push_back(std::format(format, args...));
	}
	
	bool is_token_operator()						{ return (current()->flags & TypeFlag_Op); }
	bool is_token_keyword()							{ return (current()->flags & TypeFlag_Keyword); }
	bool is_token_keyword_type()					{ return (current()->flags & TypeFlag_KeywordType); }
	bool is_token_static_value()					{ return (current()->flags & TypeFlag_StaticValue); }
	bool is_current(TokenID id)						{ return (current_token_id() == id); }
	bool is_next(TokenID id)						{ return (next_token() == id); }
	bool is(Token* token, TokenID id)				{ return (token->id == id); }
	bool eof()										{ return tokens.empty(); }

	Token* push_and_pop();
	Token* eat_expect(TokenID expected_token);
	Token* eat_expect_keyword_declaration();
	Token* eat();
	Token* eat_if_current_is(TokenID id)			{ return (is_current(id) ? eat() : nullptr); }
	Token* eat_if_current_is_type()					{ return (is_token_keyword_type() ? eat() : nullptr); }
	Token* eat_if_current_is_keyword()				{ return (is_token_keyword() ? eat() : nullptr); }
	Token* eat_if_current_is_static_value()			{ return (is_token_static_value() ? eat() : nullptr); }
	Token* current() const							{ return (tokens.empty() ? nullptr : tokens.back()); }

	TokenID current_token_id() const				{ return (tokens.empty() ? Token_Eof : tokens.back()->id); }
	TokenID next_token() const						{ return (tokens.size() < 2 ? Token_Eof : (*(tokens.rbegin() + 1))->id); }
		
	const size_t get_tokens_count() const			{ return tokens.size(); }

	// static methods

	static inline std::string STRIFY_TOKEN(TokenID id)
	{
		switch (id)
		{
		case Token_Id:					return "Token_Id";
		case Token_Keyword:				return "Token_Keyword";
		case Token_IntLiteral:			return "Token_IntLiteral";
		case Token_Comment:				return "Token_Comment";
		case Token_ShrAssign:			return "Token_ShrAssign";
		case Token_ShlAssign:			return "Token_ShlAssign";
		case Token_AddAssign:			return "Token_AddAssign";
		case Token_Inc:					return "Token_Inc";
		case Token_Dec:					return "Token_Dec";
		case Token_SubAssign:			return "Token_SubAssign";
		case Token_MulAssign:			return "Token_MulAssign";
		case Token_ModAssign:			return "Token_ModAssign";
		case Token_DivAssign:			return "Token_DivAssign";
		case Token_OrAssign:			return "Token_OrAssign";
		case Token_AndAssign:			return "Token_AndAssign";
		case Token_XorAssign:			return "Token_XorAssign";
		case Token_Gte:					return "Token_Gte";
		case Token_Lte:					return "Token_Lte";
		case Token_Colon:				return "Token_Colon";
		case Token_Semicolon:			return "Token_Semicolon";
		case Token_Comma:				return "Token_Comma";
		case Token_ParenOpen:			return "Token_ParenOpen";
		case Token_ParenClose:			return "Token_ParenClose";
		case Token_BraceOpen:			return "Token_BraceOpen";
		case Token_BraceClose:			return "Token_BraceClose";
		case Token_BracketOpen:			return "Token_BracketOpen";
		case Token_BracketClose:		return "Token_BracketClose";
		case Token_Add:					return "Token_Add";
		case Token_Sub:					return "Token_Sub";
		case Token_Mul:					return "Token_Mul";
		case Token_Mod:					return "Token_Mod";
		case Token_Div:					return "Token_Div";
		case Token_Shr:					return "Token_Shr";
		case Token_Shl:					return "Token_Shl";
		case Token_And:					return "Token_And";
		case Token_Or:					return "Token_Or";
		case Token_LogicalNot:			return "Token_LogicalNot";
		case Token_LogicalAnd:			return "Token_LogicalAnd";
		case Token_LogicalOr:			return "Token_LogicalOr";
		case Token_Xor:					return "Token_Xor";
		case Token_Not:					return "Token_Not";
		case Token_Assign:				return "Token_Assign";
		case Token_Equal:				return "Token_Equal";
		case Token_NotEqual:			return "Token_NotEqual";
		case Token_Gt:					return "Token_Gt";
		case Token_Lt:					return "Token_Lt";
		case Token_Void:				return "Token_Void";
		case Token_Bool:				return "Token_Bool";
		case Token_U8:					return "Token_U8";
		case Token_U16:					return "Token_U16";
		case Token_U32:					return "Token_U32";
		case Token_U64:					return "Token_U64";
		case Token_I8:					return "Token_I8";
		case Token_I16:					return "Token_I16";
		case Token_I32:					return "Token_I32";
		case Token_I64:					return "Token_I64";
		case Token_M128:				return "Token_M128";
		case Token_For:					return "Token_For";
		case Token_While:				return "Token_While";
		case Token_If:					return "Token_If";
		case Token_Else:				return "Token_Else";
		case Token_Break:				return "Token_Break";
		case Token_Continue:			return "Token_Continue";
		case Token_Return:				return "Token_Return";
		case Token_Extern:				return "Token_Extern";
		case Token_True:				return "Token_True";
		case Token_False:				return "Token_False";
		case Token_Eof:					return "Token_Eof";
		}

		return "Token_None";
	}

	static inline std::string STRIFY_TOKEN(Token* token)
	{
		return STRIFY_TOKEN(token->id);
	}
};

inline std::unique_ptr<Lexer> g_lexer;