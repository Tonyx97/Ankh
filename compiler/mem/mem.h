#pragma once

#include <memory>
#include <source_location>
#include <string>

namespace mem
{
	struct AllocInfo
	{
		std::string src,
					type_name;

		void* address;

		size_t size;

		int line,
			column;

		bool pool;
	};

	void throw_alloc_error();
	void throw_collision_error();
	void throw_missing_error();
	void save_allocation(std::source_location src, const std::string& type_name, void* instance, size_t size, bool pool);
	void save_free(void* instance);
	bool check_and_dump_memory_leaks();
	
	template <typename T, typename... A>
	T* alloc(const std::source_location& src, const A&... args)
	{
		auto instance = new(std::nothrow) T(std::forward<const A&>(args)...);
		if (!instance)
			throw_alloc_error();

		save_allocation(src, typeid(T).name(), instance, sizeof(T), false);

		return instance;
	}

	template <typename T>
	T* alloc_pool(const std::source_location& src, size_t size)
	{
		auto instance = new(std::nothrow) T[size]();
		if (!instance)
			throw_alloc_error();

		save_allocation(src, typeid(T).name(), instance, sizeof(T) * size, true);

		return instance;
	}

	template <typename T>
	void free(T* instance)
	{
		if (!instance)
			return;

		save_free(instance);

		delete instance;
	}

	template <typename T>
	void free_pool(T* instance)
	{
		if (!instance)
			return;

		save_free(instance);

		delete[] instance;
	}
}

#define _ALLOC(type, ...)			mem::alloc<type>(std::source_location::current(), __VA_ARGS__)
#define _ALLOC_POOL(type, size)		mem::alloc_pool<type>(std::source_location::current(), size)
#define _FREE						mem::free
#define _FREE_R(x)					{ mem::free(x); x = nullptr; }
#define _FREE_POOL					mem::free_pool