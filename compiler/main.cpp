#include <defs.h>

#include <lexer/lexer.h>
#include <syntax/syntax.h>
#include <semantic/semantic.h>

int main()
{
	setup_console();

	g_lexer = std::make_unique<Lexer>();
	g_syntax = std::make_unique<Syntax>();
	g_semantic = std::make_unique<Semantic>();

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

	PRINT(Cyan, "\n---------- Semantic Analysis ----------");

	if (!g_semantic->run())
	{
		g_semantic->print_errors();

		goto finish_compiling;
	}

finish_compiling:

	g_semantic.reset();
	g_syntax.reset();
	g_lexer.reset();

	if (!mem::check_and_dump_memory_leaks())
		std::cin.get();

	return std::cin.get();
}