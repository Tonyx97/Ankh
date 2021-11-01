#pragma once

#include <enums/ast_ir_types.h>

namespace ir
{
	struct Type
	{
		TypeID type = TypeID::Type_None;
		int indirection = 0;

		std::string indirection_str() const
		{
			if (indirection <= 0)
				return {};

			std::string str;

			str.resize(indirection);

			std::fill_n(str.begin(), indirection, '*');

			return str;
		}

		std::string str() const					{ return STRIFY_TYPE(type); }
		std::string full_str() const			{ return str() + indirection_str(); }
		Type deref() { return Type{ type, indirection - 1 }; }
		Type ref() { return Type{ type, indirection + 1 }; }

		bool is_same_type(const Type& v) const	{ return type == v.type && indirection == v.indirection; }
		bool is_same_type(TypeID v) const		{ return type == v && indirection == 0; }
		bool operator == (const Type& v) const	{ return is_same_type(v); }
	};
}