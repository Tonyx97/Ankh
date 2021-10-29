#include <defs.h>

#include "lexer.h"

Lexer::~Lexer()
{
	for (auto token : tokens)		_FREE(token);
	for (auto token : eaten_tokens) _FREE(token);
}

bool Lexer::run(const std::string& filename)
{
	// open the input file

	auto file = std::ifstream(filename);
	if (!file)
		return false;

	// parse each line of the input file

	std::string line;

	auto remove_spaces = [&]()
	{
		if (line.empty())
			return;

		if (line.find_first_not_of("\t ") != std::string::npos)
			for (auto curr_char = line.at(0); (curr_char == ' ' || curr_char == '\t'); curr_char = line.at(0))
				line = line.substr(line.find_first_not_of("\t "), std::string::npos);
		else line.clear();
	};

	auto strip_line = [&](size_t off, size_t count)
	{
		return (line = line.erase(off, count)).empty();
	};
	
	auto next = [&](size_t off)
	{
		line = line.substr(off, std::string::npos);
		remove_spaces();
	};

	int line_num = 1,
		multiline_comment = 0;

	while (std::getline(file, line))
	{
		// remove spaces and tabs at the beginning of the line

		remove_spaces();
		
		while (!line.empty())
		{
			auto multi_comment_begin_pos = line.find("/*"),
				 multi_comment_end_pos = line.find("*/");

			if (multi_comment_begin_pos != std::string::npos &&
				multi_comment_end_pos != std::string::npos)
			{
				if (strip_line(multi_comment_begin_pos, multi_comment_end_pos - multi_comment_begin_pos + 2))
					break;
			}
			else
			{
				if (multi_comment_end_pos != std::string::npos)
				{
					if (--multiline_comment < 0)
						multiline_comment = 0;

					if (strip_line(0, multi_comment_end_pos + 2))
						break;
				}
				else if (multi_comment_begin_pos == 0)
				{
					++multiline_comment;
					break;
				}
			}

			if (multiline_comment)
				break;

			std::smatch sm;

			if (std::regex_search(line, sm, regex::SINGLE_COMMENT) &&
				line.find(sm.str()) == 0)
				break;

			auto curr_token = _ALLOC(Token);

			size_t token_len = 0;

			bool valid = false;

			auto check_token_regex = [&](const std::regex& rgx, TokenID token_type)
			{
				if (std::regex_search(line, sm, rgx))
				{
					if (auto token_found = sm.str(); valid = (line.find(token_found) == 0))
					{
						token_len = token_found.length();

						switch (token_type)
						{
						case Token_Id:
						{
							if (auto it = g_keywords.find(token_found); it != g_keywords.end())
							{
								curr_token->value = token_found;
								curr_token->id = it->second;
								curr_token->flags |= TokenFlag_Keyword;
							}
							else if (auto it_decl = g_keywords_type.find(token_found); it_decl != g_keywords_type.end())
							{
								curr_token->value = token_found;
								curr_token->id = std::get<0>(it_decl->second);
								curr_token->flags |= TokenFlag_KeywordType | std::get<2>(it_decl->second);
							}
							else if (auto it_static_val = g_static_values.find(token_found); it_static_val != g_static_values.end())
							{
								switch (it_static_val->second)
								{
								case Token_U8:
								{
									curr_token->value = token_found == "true" ? "1" : "0";
									curr_token->id = Token_U8;
									curr_token->flags |= TokenFlag_Unsigned;
									break;
								}
								}
							}
							else
							{
								curr_token->value = token_found;
								curr_token->id = token_type;
								curr_token->flags |= TokenFlag_Id;
							}

							break;
						}
						case Token_IntLiteral:
						{
							curr_token->id = token_type;

							const auto int_group = sm[1],
									   unsigned_group = sm[3],
									   size_group = sm[4];

							if (unsigned_group.matched && size_group.matched)
							{
								const bool is_unsigned = unsigned_group.str() == "u";

								curr_token->flags |= is_unsigned ? TokenFlag_Unsigned : 0;

								switch (std::stoll(size_group))
								{
								case 0:		curr_token->id = Token_Void; break;
								case 8:		curr_token->id = is_unsigned ? Token_U8  : Token_I8;   break;
								case 16:	curr_token->id = is_unsigned ? Token_U16 : Token_I16;  break;
								case 32:	curr_token->id = is_unsigned ? Token_U32 : Token_I32;  break;
								case 64:	curr_token->id = is_unsigned ? Token_U64 : Token_I64;  break;
								}
							}
							else curr_token->id = Token_I32;

							curr_token->value = token_found;

							break;
						}
						}

						return true;
					}
				}

				return false;
			};

			check_token_regex(regex::INT_LITERAL, Token_IntLiteral);

			for (const auto& token : g_static_tokens)
			{
				const auto len = token.value.length();

				if (!line.compare(0, len, token.value))
				{
					*curr_token = token;
					valid = true;
					token_len = len;
					break;
				}
			}

			if (!valid)
				check_token_regex(regex::WORD, Token_Id);

			if (valid)
			{
				curr_token->line = line_num;

				tokens.push_back(curr_token);

				next(token_len);
			}
			else
			{
				auto invalid_token = line;
				
				if (auto next_space = invalid_token.find_first_of("\t "); next_space != std::string::npos)
				{
					invalid_token = invalid_token.substr(0, next_space);
					next(next_space);
				}
				else next(line.length());
				
				add_error("{}:{} -> Unrecognized token '{}'", filename, line_num, invalid_token);
			}
		}

		++line_num;
	}

	std::reverse(tokens.begin(), tokens.end());

	return true;
}

void Lexer::print_list()
{
	PRINT_NL;

	for (auto token : tokens)
		PRINT_EX(Green, std::format("'{}' ", token->value), White, "(", Yellow, std::format("{}", STRIFY_TOKEN(token->id)), White, ")");
}

void Lexer::print_errors()
{
	for (const auto& err : errors)
		PRINT(Red, "{}", err);
}

Token* Lexer::push_and_pop()
{
	check(!tokens.empty(), "EOF");

	auto curr = current();

	eaten_tokens.push_back(curr);
	tokens.pop_back();

	return curr;
}

Token* Lexer::eat_expect(TokenID expected_token)
{
	check(!eof(), "Expected a keyword, EOF found");

	auto curr = current();

	check(curr->id == expected_token, "Unexpected token '{}'", curr->value);

	return push_and_pop();
}

Token* Lexer::eat_expect_keyword_declaration()
{
	check(!eof(), "Expected a keyword, EOF found");

	auto curr = current();

	check(curr->flags & TokenFlag_KeywordType, "Unexpected token '{}'", curr->value);

	return push_and_pop();
}

Token* Lexer::eat()
{
	check(!eof(), "Expected a keyword, EOF found");

	return push_and_pop();
}

Token* Lexer::eat_if_current_is_int_literal()
{
	switch (current_token_id())
	{
	case Token_U8:
	case Token_U16:
	case Token_U32:
	case Token_U64:
	case Token_I8:
	case Token_I16:
	case Token_I32:
	case Token_I64: return eat();
	}

	return nullptr;
}