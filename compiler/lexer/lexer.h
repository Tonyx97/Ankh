#pragma once

enum TokenID : int
{
	Token_Id,
	Token_Keyword,
	Token_IntLiteral,
	Token_Comment,

	Token_ShrAssign,
	Token_ShlAssign,
	Token_AddAssign,
	Token_Inc,
	Token_Dec,
	Token_SubAssign,
	Token_MulAssign,
	Token_ModAssign,
	Token_DivAssign,
	Token_OrAssign,
	Token_AndAssign,
	Token_XorAssign,
	Token_Gte,
	Token_Lte,

	Token_Colon,
	Token_Semicolon,
	Token_Comma,

	Token_ParenOpen,
	Token_ParenClose,
	Token_BraceOpen,
	Token_BraceClose,
	Token_BracketOpen,
	Token_BracketClose,
	Token_Add,
	Token_Sub,
	Token_Mul,
	Token_Mod,
	Token_Div,
	Token_Shr,
	Token_Shl,
	Token_And,
	Token_Or,
	Token_Xor,
	Token_Not,
	Token_Assign,

	Token_LogicalNot,
	Token_LogicalAnd,
	Token_LogicalOr,

	Token_Equal,
	Token_NotEqual,
	Token_Gt,
	Token_Lt,

	Token_Void,
	Token_Bool,
	Token_True,
	Token_False,
	Token_U8,
	Token_U16,
	Token_U32,
	Token_U64,
	Token_I8,
	Token_I16,
	Token_I32,
	Token_I64,
	Token_M128,

	Token_For,
	Token_While,
	Token_If,
	Token_Else,
	Token_Break,
	Token_Continue,
	Token_Return,
	Token_Extern,

	Token_Eof,
	Token_None
};

enum TokenFlag : unsigned int
{
	TokenFlag_None			= 0,
	TokenFlag_Op			= (1 << 0),
	TokenFlag_Keyword		= (1 << 1),
	TokenFlag_KeywordType	= (1 << 2),
	TokenFlag_Unsigned		= (1 << 3),
	TokenFlag_Assignation	= (1 << 4),
};

struct Token
{
	static constexpr int LOWEST_PRECEDENCE = 16;

	std::string value = "";

	struct
	{
		uint64_t int_value = 0;

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
	};

	TokenID id = Token_None;

	uint32_t flags = TokenFlag_None;

	int precedence = LOWEST_PRECEDENCE,
		line = 0;

	bool valid = false;
	
	int8_t size = 0;

	Token& operator = (const Token& token)
	{
		if (this != std::addressof(token))
		{
			value = token.value;
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
	{ "if",			Token_If },
	{ "else",		Token_Else },
	{ "break",		Token_Break },
	{ "continue",	Token_Continue },
	{ "return",		Token_Return },
	{ "extern",		Token_Extern },		// not an statement but we put it here for now
};

inline std::unordered_map<std::string, TokenID> g_keywords_type =
{
	{ "void",	Token_Void },
	{ "bool",	Token_Bool },
	{ "true",	Token_True },
	{ "false",	Token_False },
	{ "u8",		Token_U8 },
	{ "u16",	Token_U16 },
	{ "u32",	Token_U32 },
	{ "u64",	Token_U64 },
	{ "i8",		Token_I8 },
	{ "i16",	Token_I16 },
	{ "i32",	Token_I32 },
	{ "i64",	Token_I64 },
	{ "m128",	Token_M128 },
};

inline Token g_static_tokens[] =
{
	{ .value = ">>=", .id = Token_ShrAssign, .flags = TokenFlag_Op | TokenFlag_Assignation, .precedence = 14 },
	{ .value = "<<=", .id = Token_ShlAssign, .flags = TokenFlag_Op | TokenFlag_Assignation, .precedence = 14 },

	{ .value = "==", .id = Token_Equal,			.flags = TokenFlag_Op,							.precedence = 7 },
	{ .value = "!=", .id = Token_NotEqual,		.flags = TokenFlag_Op,							.precedence = 7 },
	{ .value = ">=", .id = Token_Gte,			.flags = TokenFlag_Op,							.precedence = 6 },
	{ .value = "<=", .id = Token_Lte,			.flags = TokenFlag_Op,							.precedence = 6 },
	{ .value = "+=", .id = Token_AddAssign,		.flags = TokenFlag_Op | TokenFlag_Assignation,	.precedence = 14 },
	{ .value = "-=", .id = Token_SubAssign,		.flags = TokenFlag_Op | TokenFlag_Assignation,	.precedence = 14 },
	{ .value = "++", .id = Token_Inc,			.flags = TokenFlag_Op,							.precedence = 1 },
	{ .value = "--", .id = Token_Dec,			.flags = TokenFlag_Op,							.precedence = 1 },
	{ .value = "*=", .id = Token_MulAssign,		.flags = TokenFlag_Op | TokenFlag_Assignation,	.precedence = 14 },
	{ .value = "%=", .id = Token_ModAssign,		.flags = TokenFlag_Op | TokenFlag_Assignation,	.precedence = 14 },
	{ .value = "/=", .id = Token_DivAssign,		.flags = TokenFlag_Op | TokenFlag_Assignation,	.precedence = 14 },
	{ .value = "&=", .id = Token_AndAssign,		.flags = TokenFlag_Op | TokenFlag_Assignation,	.precedence = 14 },
	{ .value = "|=", .id = Token_OrAssign,		.flags = TokenFlag_Op | TokenFlag_Assignation,	.precedence = 14 },
	{ .value = "^=", .id = Token_XorAssign,		.flags = TokenFlag_Op | TokenFlag_Assignation,	.precedence = 14 },
	{ .value = "&&", .id = Token_LogicalAnd,	.flags = TokenFlag_Op,							.precedence = 11 },
	{ .value = "||", .id = Token_LogicalOr,		.flags = TokenFlag_Op,							.precedence = 12 },
	{ .value = ">>", .id = Token_Shr,			.flags = TokenFlag_Op,							.precedence = 5 },
	{ .value = "<<", .id = Token_Shl,			.flags = TokenFlag_Op,							.precedence = 5 },

	{ .value = ";", .id = Token_Semicolon },
	{ .value = ",", .id = Token_Comma,		.flags = TokenFlag_None,						.precedence = 15 },
	{ .value = "(", .id = Token_ParenOpen,	.flags = TokenFlag_None,						.precedence = 1 },
	{ .value = ")", .id = Token_ParenClose,	.flags = TokenFlag_None,						.precedence = 1 },
	{ .value = "{", .id = Token_BracketOpen },
	{ .value = "}", .id = Token_BracketClose },
	{ .value = "[", .id = Token_BraceOpen,	.flags = TokenFlag_Op,							.precedence = 1 },
	{ .value = "]", .id = Token_BraceClose,	.flags = TokenFlag_Op,							.precedence = 1 },
	{ .value = "+", .id = Token_Add,		.flags = TokenFlag_Op,							.precedence = 4 },
	{ .value = "-", .id = Token_Sub,		.flags = TokenFlag_Op,							.precedence = 4 },
	{ .value = "*", .id = Token_Mul,		.flags = TokenFlag_Op,							.precedence = 3 },
	{ .value = "%", .id = Token_Mod,		.flags = TokenFlag_Op,							.precedence = 3 },
	{ .value = "/", .id = Token_Div,		.flags = TokenFlag_Op,							.precedence = 3 },
	{ .value = "&", .id = Token_And,		.flags = TokenFlag_Op,							.precedence = 8 },
	{ .value = "|", .id = Token_Or,			.flags = TokenFlag_Op,							.precedence = 10 },
	{ .value = "^", .id = Token_Xor,		.flags = TokenFlag_Op,							.precedence = 9 },
	{ .value = "~", .id = Token_Not,		.flags = TokenFlag_Op,							.precedence = 2 },
	{ .value = "!", .id = Token_LogicalNot,	.flags = TokenFlag_Op,							.precedence = 2 },
	{ .value = "=", .id = Token_Assign,		.flags = TokenFlag_Op | TokenFlag_Assignation,	.precedence = 14 },
	{ .value = ">", .id = Token_Gt,			.flags = TokenFlag_Op,							.precedence = 6 },
	{ .value = "<", .id = Token_Lt,			.flags = TokenFlag_Op,							.precedence = 6 },
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
	void push_and_pop_token(Token* token);

	template <typename... A>
	inline void add_error(const std::string& format, A... args)
	{
		errors.push_back(std::format(format, args...));
	}
	
	bool is_token_operator()						{ return (current()->flags & TokenFlag_Op); }
	bool is_token_keyword()							{ return (current()->flags & TokenFlag_Keyword); }
	bool is_token_keyword_type()					{ return (current()->flags & TokenFlag_KeywordType); }
	bool is_current(TokenID id)						{ return (current_token_id() == id); }
	bool is_next(TokenID id)						{ return (next_token() == id); }
	bool is(Token* token, TokenID id)				{ return (token->id == id); }
	bool eof()										{ return tokens.empty(); }

	Token* eat_expect(TokenID expected_token);
	Token* eat_expect_keyword_declaration();
	Token* eat();
	Token* current() const							{ return (tokens.empty() ? nullptr : tokens.back()); }

	TokenID current_token_id() const				{ return (tokens.empty() ? Token_Eof : tokens.back()->id); }
	TokenID next_token() const						{ return (tokens.size() < 2 ? Token_Eof : (*(tokens.rbegin() + 1))->id); }
		
	const size_t get_tokens_count() const			{ return tokens.size(); }

	// static methods
	
	static inline std::string STRIFY_TYPE(TokenID id)
	{
		auto it = std::find_if(g_keywords_type.begin(), g_keywords_type.end(), [&](const auto& p) { return p.second == id; });
		return (it != g_keywords_type.end() ? it->first : "unknown_type");
	}

	static inline std::string STRIFY_TYPE(Token* token)
	{
		return STRIFY_TYPE(token->id);
	}

	static inline std::string STRIFY_OPERATOR(TokenID id)
	{
		for (auto&& token : g_static_tokens)
			if (token.id == id)
				return token.value;

		return "unknown_op";
	}

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
		case Token_Eof:					return "Token_Eof";
		}

		return "TOKEN_NONE";
	}

	static inline std::string STRIFY_TOKEN(Token* token)
	{
		return STRIFY_TOKEN(token->id);
	}
};

inline std::unique_ptr<Lexer> g_lexer;