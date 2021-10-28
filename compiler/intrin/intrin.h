#pragma once

class Intrinsic
{
private:

	std::unordered_set<std::string> intrinsics;

public:

	Intrinsic() {}

	bool is_intrinsic(const std::string& name);
};

inline std::unique_ptr<Intrinsic> g_intrin;