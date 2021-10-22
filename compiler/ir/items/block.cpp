#include <defs.h>

#include "block.h"

namespace ir
{
	Block::~Block()
	{
		for (auto item : items)
			_FREE(item);
	}

	void Block::print()
	{
		if (!is_entry())
			PRINT_NL;

		PRINT(Grey, name + ":");

		for (auto item : items)
			item->print();
	}
}