#include <defs.h>

#include <lexer/lexer.h>
#include <syntax/syntax.h>

int main()
{
	setup_console();

	g_lexer = std::make_unique<Lexer>();
	g_syntax = std::make_unique<Syntax>();

	PRINT(Cyan, "---------- Lexic Analysis ----------");

	g_lexer->run("test.ankh");
	g_lexer->print_errors();
	g_lexer->print_list();

	PRINT(Cyan, "\n---------- Syntax Analysis ----------");

	{
		PROFILE("Syntax Time");
		g_syntax->run();
	}

	PRINT(Cyan, "\n---------- AST ----------");

	g_syntax->print_ast();

	g_syntax.reset();
	g_lexer.reset();

	if (!mem::check_and_dump_memory_leaks())
		std::cin.get();

	return std::cin.get();
}