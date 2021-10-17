#pragma once

namespace rtti
{
	template <typename T, typename Ty>
	T* cast(Ty ptr)
	{
		if (!ptr)
			return nullptr;

		return (T::check_class(ptr) ? static_cast<T*>(ptr) : nullptr);
	}
}