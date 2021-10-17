#include <defs.h>

#include "mem.h"

namespace mem
{
	std::unordered_map<void*, AllocInfo> engine_allocs;

	std::mutex mem_mtx;
	
	size_t total_instance_allocs = 0,
		   total_pool_allocs = 0,
		   total_instance_frees = 0,
		   total_pool_frees = 0;

	void throw_alloc_error()
	{
		size_t allocated_memory = 0;

		for (const auto& [ptr, mem_info] : engine_allocs)
			allocated_memory += mem_info.size;

		PRINT(Red, "Engine halted due to lack of virtual memory (allocated memory %.3f GiB)", static_cast<float>(allocated_memory) / (1024.f * 1024.f * 1024.f));
	}

	void throw_collision_error()
	{
		PRINT(Red, "Engine halted due to memory collision");
	}

	void throw_missing_error()
	{
		PRINT(Red, "Engine halted due to an attempt of freeing already-freed memory");
	}

	void save_allocation(std::source_location src, const std::string& type_name, void* instance, size_t size, bool pool)
	{
		std::lock_guard lock(mem_mtx);

		if (engine_allocs.find(instance) == engine_allocs.end())
		{
			engine_allocs.insert({ instance,
				{
					.src = std::string(src.file_name()),
					.type_name = type_name,
					.address = instance,
					.size = size,
					.line = int(src.line()),
					.column = int(src.column()),
					.pool = pool
				} });
		}
		else throw_collision_error();

		//prof::log(YELLOW, "Allocated '{}' from '{}' at {}", type_name, src.file_name(), instance);
	}

	void save_free(void* instance)
	{
		std::lock_guard lock(mem_mtx);

		if (auto it = engine_allocs.find(instance); it != engine_allocs.end())
			engine_allocs.erase(it);
		else throw_missing_error();
	}

	bool check_and_dump_memory_leaks()
	{
		if (engine_allocs.empty())
			return (total_instance_allocs == total_instance_frees && total_pool_allocs == total_pool_frees);

		std::string info;

		for (const auto& [ptr, i] : engine_allocs)
			info += std::format("'{}' | line {}:{} ['{}' - {}]\n\n", i.src, i.line, i.column, i.type_name, i.address);

		PRINT(Red, "Memory leak detected.\n\n{}", info);

		return false;
	}
}