#pragma once

#include <Windows.h>

#include "defs.h"

enum ColorType : unsigned short
{
	Black = 0x0,
	DarkBlue = 0x1,
	DarkGreen = 0x2,
	DarkCyan = 0x3,
	DarkRed = 0x4,
	DarkPurple = 0x5,
	DarkYellow = 0x6,
	Grey = 0x7,
	DarkGrey = 0x8,
	Blue = 0x9,
	Green = 0xA,
	Cyan = 0xB,
	Red = 0xC,
	Purple = 0xD,
	Yellow = 0xE,
	White = 0xF,
};

static constexpr auto SINGLE_TEXT_MAX_LENGTH = 0x800;
static constexpr auto SPACES_PER_TAB = 2;

enum TextSection
{
	HEADER,
	FOOTER,
};

class basic_buffer
{
private:

	char* data = nullptr;

public:

	basic_buffer(size_t max_len)	{ data = new char[max_len](); }
	~basic_buffer()					{ delete[] data; }
		
	char* get()						{ return data; }

	friend std::ostream& operator << (std::ostream& os, const basic_buffer& buffer)
	{
		std::cout << buffer.data;
		return os;
	}
};

struct color
{
	color(uint16_t value)	{ SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), value); }
	~color()				{ SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xF); }
};

class text
{
private:

	std::string data;

	uint16_t color_id;

	int alignment = 0;

	bool nl;

public:

	text(uint16_t color_id, const std::string& data, bool nl, int alignment = 0) : data(data), color_id(color_id), nl(nl), alignment(alignment) {}

	void print() { std::cout << *this; }

	friend std::ostream& operator << (std::ostream& os, const text& t)
	{
		auto align = [&](int width)
		{
			std::cout.setf(std::ios_base::left, std::ios_base::adjustfield);
			std::cout.fill(' ');
			std::cout.width(width);
		};

		if (t.alignment > 0)
			align(t.alignment);

		auto new_line = (t.nl ? "\n" : "");

		color c(t.color_id);

		std::cout << t.data << new_line;

		return os;
	}
};

inline void setup_console()
{
	static constexpr auto console_size_x = 800,
							console_size_y = 800;

	SetConsoleOutputCP(CP_UTF8);
	SetWindowPos(GetConsoleWindow(), nullptr, GetSystemMetrics(SM_CXSCREEN) / 2 - console_size_x / 2, GetSystemMetrics(SM_CYSCREEN) / 2 - console_size_y / 2, console_size_x, console_size_y, 0);
}

template <typename... A>
static inline text make_text(uint16_t color, const std::string& txt, A&&... args)
{
	return text(color, std::format(txt, args...), false);
}

template <typename... A>
static inline text make_text_nl(uint16_t color, const std::string& txt, A&&... args)
{
	return text(color, std::format(txt, args...), true);
}

template <typename... A>
static inline text make_text_align(uint16_t color, const std::string& txt, int alignment, A&&... args)
{
	return text(color, std::format(txt, args...), false, alignment);
}

template <typename... A>
static inline text make_text_with_tabs(uint16_t color, const std::string& txt, int tabs, A&&... args)
{
	std::string tabs_str(tabs * SPACES_PER_TAB, 0);

	std::generate(tabs_str.begin(), tabs_str.end(), [] { return ' '; });

	return text(color, tabs_str + std::format(txt, args...), false, 0);
}

template <typename... A>
static inline void print(uint16_t color_id, const std::string& txt, A&&... args)
{
	color c(color_id);
	std::cout << std::format(txt, args...);
}

template <typename... A>
static inline void println(uint16_t color_id, const std::string& txt, A&&... args)
{
	color c(color_id);
	std::cout << std::format(txt, args...) << std::endl;
}

template <typename Tx, typename T, typename F>
static inline void print_vec_int(ColorType color, const std::vector<T>& vec, const std::string& separator, const F& fn)
{
	if (vec.empty())
		return;

	for (int i = 0; i < vec.size() - 1; ++i)
	{
		make_text(color, "{}", fn(static_cast<Tx>(vec[i])).c_str()).print();
		make_text(White, "{}", separator.c_str()).print();
	}

	make_text(color, "{}", fn(static_cast<Tx>(vec.back())).c_str()).print();
}

template <typename Tx, typename T, typename F>
static inline void print_vec(ColorType color, const std::vector<T>& vec, const std::string& separator, const F& fn)
{
	if (vec.empty())
		return;

	for (int i = 0; i < vec.size() - 1; ++i)
	{
		make_text(color, "{}", fn(static_cast<Tx*>(vec[i])).c_str()).print();
		make_text(White, "{}", separator.c_str()).print();
	}

	make_text(color, "{}", fn(static_cast<Tx*>(vec.back())).c_str()).print();
}

template <typename Tx, typename T, typename F>
static inline void print_set(ColorType color, const std::set<T>& set, const std::string& separator, const F& fn)
{
	if (set.empty())
	{
		make_text(color, "EMPTY").print();
		return;
	}

	auto set_size = set.size(),
			i = 0ull;

	for (auto it = set.begin(); it != set.end(); ++it)
	{
		if (i++ == set_size - 1)
			make_text(color, "{}", fn(*set.rbegin()).c_str()).print();
		else make_text(color, "{}{}", fn(*it).c_str(), separator.c_str()).print();
	}
}

template <typename Tx, typename T, typename F>
static inline void print_set(ColorType color, const std::unordered_set<T>& set, const std::string& separator, const F& fn)
{
	if (set.empty())
	{
		make_text(color, "EMPTY").print();
		return;
	}

	auto set_size = set.size(),
			i = 0ull;

	std::vector<T> temp;

	for (const auto& e : set)
		temp.push_back(e);

	print_vec<Tx, T, F>(color, temp, separator, fn);
}

struct TimeProfiling
{
	std::chrono::high_resolution_clock::time_point m_start;
	uint64_t cycles = 0;
	std::string name;

	TimeProfiling(const std::string& name) : name(name)
	{
		m_start = std::chrono::high_resolution_clock::now();
		cycles = __rdtsc();
	}

	~TimeProfiling()
	{
		const auto cycles_passed = __rdtsc() - cycles;
		const auto time_passed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_start).count();
		make_text_nl(Yellow, "{}: {:.3f} ms | {} mcs | {} cycles", name.c_str(), static_cast<double>(time_passed) / 1000.f, time_passed, cycles_passed).print();
	}
};

#define PROFILE(x)						TimeProfiling p(x)
#define EMPTY_NEW_LINE					White, "\n"
#define PRINT_NNL(x, y, ...)			make_text(x, y, __VA_ARGS__).print()
#define PRINT(x, y, ...)				make_text_nl(x, y, __VA_ARGS__).print()
#define PRINT_ALIGN(x, y, z, ...)		make_text_align(x, z, y, __VA_ARGS__).print()
#define PRINT_ALIGN_NL(x, y, z, ...)	make_text_align(x, z, y, __VA_ARGS__).print(); make_text(EMPTY_NEW_LINE).print()
#define PRINT_TABS(x, y, z, ...)		make_text_with_tabs(x, z, y, __VA_ARGS__).print()
#define PRINT_TABS_NL(x, y, z, ...)		make_text_with_tabs(x, z, y, __VA_ARGS__).print(); make_text(EMPTY_NEW_LINE).print()
#define PRINT_NL						make_text(EMPTY_NEW_LINE).print()

class compiler_exception : public std::exception
{
private:

	std::string msg;

public:

	compiler_exception(const std::string& msg) : msg(msg), exception() {}

	~compiler_exception() throw() {}

	const char* what() const throw () { return msg.c_str(); }
};

template <typename... A>
inline void global_error(const char* text, A... args)
{
	const std::string error_msg = std::format(text, args...);

	throw compiler_exception(error_msg);
}

template <typename... A>
inline void check(bool condition, const char* text, A... args)
{
	if (condition)
		return;

	global_error(text, args...);
}