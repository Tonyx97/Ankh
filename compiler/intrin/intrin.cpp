#include <defs.h>

#include "intrin.h"

Intrinsic::Intrinsic()
{
	intrinsics.insert("__rdtsc");
}

bool Intrinsic::is_intrinsic(const std::string& name)
{
	return intrinsics.contains(name);
}