#include <defs.h>

#include <lexer/lexer.h>

int main()
{
	setup_console();

	PRINT(C_CYAN, "---------- Lexer (Lexic Analysis) ----------\n");

	lexer lexer;

	err_lexer = &lexer;

	return std::cin.get();
}