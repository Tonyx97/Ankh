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
			
			auto check_token_regex = [&](const std::regex& rgx, TokenID token_type)
			{
				if (std::regex_search(line, sm, rgx))
				{
					if (auto token_found = sm.str(); curr_token->valid = (line.find(token_found) == 0))
					{
						switch (token_type)
						{
						case Token_Id:
						{
							if (auto it = g_keywords.find(token_found); it != g_keywords.end())
							{
								curr_token->id = it->second;
								curr_token->flags |= TokenFlag_Keyword;
							}
							else if (auto it_decl = g_keywords_type.find(token_found); it_decl != g_keywords_type.end())
							{
								curr_token->id = std::get<0>(it_decl->second);
								curr_token->size = std::get<1>(it_decl->second);
								curr_token->flags |= TokenFlag_KeywordType | std::get<2>(it_decl->second);
							}
							else if (auto it_static_val = g_static_values.find(token_found); it_static_val != g_static_values.end())
							{
								switch (std::get<0>(it_static_val->second))
								{
								case Token_True:
								{
									curr_token->id = Token_Bool;
									curr_token->u8 = 1;
									curr_token->flags |= TokenFlag_Unsigned;
									break;
								}
								case Token_False:
								{
									curr_token->id = Token_Bool;
									curr_token->u8 = 0;
									curr_token->flags |= TokenFlag_Unsigned;
									break;
								}
								}

								curr_token->size = std::get<1>(it_static_val->second);
								curr_token->flags |= TokenFlag_StaticValue;
							}
							else
							{
								curr_token->id = token_type;
								curr_token->flags |= TokenFlag_Id;
							}

							curr_token->value = token_found;

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
								curr_token->flags |= unsigned_group.str() == "u" ? TokenFlag_Unsigned : 0;
								curr_token->size = std::stoi(size_group.str());
							}
							else curr_token->size = 32;

							curr_token->value = token_found;
							curr_token->u64 = std::stoull(int_group.str());

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
				if (!line.compare(0, token.value.length(), token.value))
				{
					*curr_token = token;
					break;
				}
			}

			if (!curr_token->valid)
				check_token_regex(regex::WORD, Token_Id);

			if (curr_token->valid)
			{
				curr_token->line = line_num;

				tokens.push_back(curr_token);

				next(curr_token->value.length());
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