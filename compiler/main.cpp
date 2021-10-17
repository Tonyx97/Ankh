#include <defs.h>

#include <lexer/lexer.h>

int main()
{
	setup_console();

	PRINT(Cyan, "---------- Lexer (Lexic Analysis) ----------");

	{
		Lexer lexer;

		lexer.run("test.ankh");
		lexer.print_errors();
		lexer.print_list();
	}

	if (!mem::check_and_dump_memory_leaks())
		std::cin.get();

	return std::cin.get();
}