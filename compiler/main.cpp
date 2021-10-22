#include <defs.h>

#include <lexer/lexer.h>
#include <syntax/syntax.h>
#include <semantic/semantic.h>
#include <ir/ir.h>

int main()
{
	setup_console();

	g_lexer = std::make_unique<Lexer>();
	g_syntax = std::make_unique<Syntax>();
	g_semantic = std::make_unique<Semantic>();
	g_ir = std::make_unique<IR>();

	PRINT(Cyan, "---------- Lexic Analysis ----------");

	g_lexer->run("test.ankh");
	g_lexer->print_errors();
	g_lexer->print_list();

	PRINT(Cyan, "\n---------- Syntax Analysis ----------");

	{
		PROFILE("Syntax Time");
		g_syntax->run();
	}

	const bool semantic_ok = g_semantic->run();

	PRINT(Cyan, "\n---------- AST ----------");

	g_syntax->print_ast();

	PRINT(Cyan, "\n---------- Semantic Analysis ----------");

	if (!semantic_ok)
	{
		g_semantic->print_errors();

		goto finish;
	}

	PRINT(Cyan, "\n---------- IR Generation ----------");

	{
		PROFILE("IR Time");
		g_ir->run();
	}

	PRINT(Cyan, "\n---------- IR ----------\n");

finish:

	g_ir.reset();
	g_semantic.reset();
	g_syntax.reset();
	g_lexer.reset();

	if (!mem::check_and_dump_memory_leaks())
		std::cin.get();

	return std::cin.get();
}