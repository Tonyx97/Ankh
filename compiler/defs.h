#pragma once

#include <string>
#include <algorithm>
#include <random>
#include <vector>
#include <queue>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <chrono>
#include <regex>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <any>
#include <stack>
#include <optional>
#include <functional>
#include <future>
#include <ranges>
#include <span>

using optional_str = std::optional<std::string>;

#include <dbg/dbg.h>

#include <rtti/rtti.h>
#include <utils/utils.h>

#ifdef _DEBUG
#pragma comment(lib, "opencv_world451d.lib")
#else
#pragma comment(lib, "opencv_world451.lib")
#endif

#include <opencv2/opencv_modules.hpp>
#include <opencv2/opencv.hpp>

#include <gv/gv.h>

#include <mem/mem.h>

using namespace std::chrono_literals;