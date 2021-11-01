#include <defs.h>

#include <intrin/intrin.h>
#include <lexer/lexer.h>
#include <syntax/syntax.h>
#include <semantic/semantic.h>
#include <ir/ir.h>

#include <ir/instructions/phi.h>

void test_ir() {
	auto prototype = new ir::Prototype("test", ir::Type{ Type_i32, 2 }, { ir::Type{ Type_i64, 0 } , ir::Type{ Type_i8, 1} });

	auto blk = prototype->create_block();
	auto ptr = blk->add_stack_alloc(ir::Type{ Type_i16, 2 });
	auto ld = blk->add_load(ptr);
	blk->add_load(ld);
	auto res = blk->add_load(prototype->params[1]);
	blk->add_store(prototype->params[1], res);
	
	auto phi = blk->add_phi(ir::Type {Type_i64, 1});

	phi->add_values(res, blk, ld, blk, ptr, blk);

	blk->add_store(prototype->params[1], res);

	prototype->print();

	std::cout << std::endl;

	delete prototype;
}

int main()
{
	setup_console();

	g_intrin = std::make_unique<Intrinsic>();
	g_lexer = std::make_unique<Lexer>();
	g_syntax = std::make_unique<Syntax>();
	g_semantic = std::make_unique<Semantic>();
	//g_ir = std::make_unique<IR>();

	PRINT(Cyan, "---------- Lexic Analysis ----------\n");

	{
		PROFILE("Lexer Time");
		g_lexer->run("test.ankh");
	}

	g_lexer->print_errors();
	g_lexer->print_list();

	PRINT(Cyan, "\n---------- Syntax Analysis ----------\n");

	{
		PROFILE("Syntax Time");
		g_syntax->run();
	}

	PRINT(Cyan, "\n---------- AST ----------\n");

	g_syntax->print_ast();

	const bool semantic_ok = g_semantic->run();


	PRINT(Cyan, "\n---------- Semantic Analysis ----------\n");

	if (!semantic_ok)
	{
		g_semantic->print_errors();

		goto finish;
	}

	PRINT(Cyan, "\n---------- AST after semantic ----------\n");

	g_syntax->print_ast();


	PRINT(Cyan, "\n---------- IR Generation ----------\n");

	{
		PROFILE("IR Time");
		//g_ir->run();
	}

	PRINT(Cyan, "\n---------- IR ----------\n");

	//g_ir->print();

	test_ir();

finish:

	//g_ir.reset();
	g_semantic.reset();
	g_syntax.reset();
	g_lexer.reset();
	g_intrin.reset();

	if (!mem::check_and_dump_memory_leaks())
		std::cin.get();

	return std::cin.get();
}