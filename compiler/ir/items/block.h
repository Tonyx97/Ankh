#pragma once

#include <ir/instructions/instruction.h>

namespace ir
{
	enum BlockFlag
	{
		BlockFlag_None = 0,
		BlockFlag_Entry,
	};

	struct Block : public ItemBase
	{
		std::vector<Instruction*> items;

		std::string name;

		uint32_t flags = BlockFlag_None;

		Block()					{ base_type = ItemType_Block; }
		~Block();

		void print();

		bool is_entry() const	{ return flags & BlockFlag_Entry; }

		template <typename T, typename... A>
		T* create_item(const A&... args)
		{
			return _ALLOC(T, args...);
		}

		template <typename T>
		T* add_item(T* item)
		{
			items.push_back(item);

			return item;
		}

		template <typename T, typename... A>
		T* add_new_item(const A&... args)
		{
			return add_item(create_item<T>(args...));
		}

		static bool check_class(ItemBase* i) { return i->base_type == ItemType_Block; }
	};
}