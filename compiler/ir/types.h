#pragma once

#include <enums/ast_ir_types.h>

namespace ir
{
	struct Type
	{
		TypeID type = Type_None;

		int indirection = 0;

		Type() {}
		Type(TypeID type, int indirection = 0) : type(type), indirection(indirection) {}

		bool is_same_type(const Type& v) const	{ return type == v.type && indirection == v.indirection; }
		bool is_same_type(TypeID v) const		{ return type == v && !is_pointer(); }
		bool is_pointer() const					{ return !(indirection == 0); }
		bool operator == (const Type& v) const	{ return is_same_type(v); }
		bool operator == (TypeID v) const		{ return is_same_type(v); }

		Type deref() const						{ return Type { type, indirection - 1 }; }
		Type ref() const						{ return Type { type, indirection + 1 }; }

		std::string str() const					{ return STRIFY_TYPE(type); }

		std::string indirection_str() const
		{
			if (indirection <= 0)
				return {};

			std::string str;

			str.resize(indirection);

			std::fill_n(str.begin(), indirection, '*');

			return str;
		}

		std::string full_str() const			{ return str() + indirection_str(); }
	};
}

namespace std
{
	template <> struct hash<ir::Type>
	{
		size_t operator()(const ir::Type& v) const { return (hash<decltype(v.type)>()(v.type) ^ hash<decltype(v.indirection)>()(v.indirection)); }
	};
}