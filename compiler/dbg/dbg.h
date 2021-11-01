#pragma once

#include <Windows.h>

struct ColorType
{
	int r, g, b;
};

static constexpr ColorType White	= { 255, 255, 255 };
static constexpr ColorType Black	= { 0, 0, 0 };
static constexpr ColorType Blue		= { 0, 0, 255 };
static constexpr ColorType Green	= { 0, 255, 0 };
static constexpr ColorType Cyan		= { 0, 255, 255 };
static constexpr ColorType Red		= { 255, 0, 0 };
static constexpr ColorType Purple	= { 255, 0, 255 };
static constexpr ColorType Yellow	= { 248, 240, 164 };
static constexpr ColorType Grey		= { 128, 128, 128 };
static constexpr ColorType LYellow	= { 255, 255, 0 };

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

class text
{
private:

	std::string data;

	ColorType color_id;

	bool nl;

public:

	text(ColorType color_id, const std::string& data, bool nl) : data(data), color_id(color_id), nl(nl) {}

	void print() { std::cout << *this; }

	friend std::ostream& operator << (std::ostream& os, const text& t)
	{
		auto new_line = (t.nl ? "\n" : "");

		std::cout << std::format("\x1b[1;38;2;{};{};{}m{}", t.color_id.r, t.color_id.g, t.color_id.b, t.data) << new_line;

		return os;
	}
};

inline void setup_console()
{
	static constexpr auto console_size_x = 800,
							console_size_y = 800;

	DWORD mode;
	GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &mode);
	mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

	SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), mode);
	SetConsoleOutputCP(CP_UTF8);
	SetWindowPos(GetConsoleWindow(), nullptr, GetSystemMetrics(SM_CXSCREEN) / 2 - console_size_x / 2, GetSystemMetrics(SM_CYSCREEN) / 2 - console_size_y / 2, console_size_x, console_size_y, 0);
}

static inline void render_text_impl(std::string& format) {}

template <typename... A>
static inline void render_text_impl(std::string& format, ColorType color, int indirection, const A&... args)
{
	for (int i = 0; i < indirection; ++i)
		format += std::format("\x1b[1;38;2;{};{};{}m{}", color.r, color.g, color.b, '*');

	render_text_impl(format, args...);
}

template <typename... A>
static inline void render_text_impl(std::string& format, ColorType color, const std::string& txt, const A&... args)
{
	format += std::format("\x1b[1;38;2;{};{};{}m{}", color.r, color.g, color.b, txt);
	render_text_impl(format, args...);
}

template <typename... A>
static inline void render_text_ex(bool nl, int spacing, A&&... args)
{
	std::string tabs_str(spacing * SPACES_PER_TAB, 0);

	std::generate(tabs_str.begin(), tabs_str.end(), [] { return ' '; });

	std::string res;

	render_text_impl(res, args...);

	auto new_line = (nl ? "\n" : "");

	std::cout << tabs_str + res << new_line;
}

template <typename... A>
static inline text make_text(ColorType color, const std::string& txt, A&&... args)
{
	return text(color, std::format(txt, args...), false);
}

template <typename... A>
static inline text make_text_nl(ColorType color, const std::string& txt, A&&... args)
{
	return text(color, std::format(txt, args...), true);
}

template <typename... A>
static inline text make_text_with_tabs(ColorType color, const std::string& txt, int tabs, A&&... args)
{
	std::string tabs_str(tabs * SPACES_PER_TAB, 0);

	std::generate(tabs_str.begin(), tabs_str.end(), [] { return ' '; });

	return text(color, tabs_str + std::format(txt, args...), false);
}

template <typename Tx, typename T, typename F>
static inline void print_vec_int(ColorType color, const std::vector<T>& vec, const std::string& separator, const F& fn)
{
	if (vec.empty())
		return;

	for (int i = 0; i < vec.size() - 1; ++i)
	{
		make_text(color, "{}", fn(static_cast<Tx>(vec[i]))).print();
		make_text(White, "{}", separator).print();
	}

	make_text(color, "{}", fn(static_cast<Tx>(vec.back()))).print();
}

template <typename Tx, typename T, typename F>
static inline void print_vec(ColorType color, const std::vector<T>& vec, const std::string& separator, const F& fn)
{
	if (vec.empty())
		return;

	for (int i = 0; i < vec.size() - 1; ++i)
	{
		make_text(color, "{}", fn(static_cast<Tx*>(vec[i]))).print();
		make_text(White, "{}", separator).print();
	}

	make_text(color, "{}", fn(static_cast<Tx*>(vec.back()))).print();
}

template <typename Tx, typename T, typename F>
static inline void fullprint_vec(ColorType color, const std::vector<T>& vec, const std::string& separator, const F& fn)
{
	if (vec.empty())
		return;

	for (int i = 0; i < vec.size() - 1; ++i)
	{
		fn(static_cast<Tx*>(vec[i]));
		make_text(White, "{}", separator).print();
	}

	fn(static_cast<Tx*>(vec.back()));
}

template <typename Tx, typename T, typename F>
static inline void fullprint_vec_obj(ColorType color, const std::vector<T>& vec, const std::string& separator, const F& fn)
{
	if (vec.empty())
		return;

	for (int i = 0; i < vec.size() - 1; ++i)
	{
		fn(vec[i]);
		make_text(White, "{}", separator).print();
	}

	fn(vec.back());
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
			make_text(color, "{}", fn(*set.rbegin())).print();
		else make_text(color, "{}{}", fn(*it), separator).print();
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
		make_text_nl(Yellow, "{}: {:.3f} ms | {} mcs | {} cycles", name, static_cast<double>(time_passed) / 1000.f, time_passed, cycles_passed).print();
	}
};

#define PROFILE(x)						TimeProfiling p(x)
#define EMPTY_NEW_LINE					White, "\n"
#define PRINT_NNL(x, y, ...)			make_text(x, y, __VA_ARGS__).print()
#define PRINT(x, y, ...)				make_text_nl(x, y, __VA_ARGS__).print()
#define PRINT_TABS(x, y, z, ...)		make_text_with_tabs(x, z, y, __VA_ARGS__).print()
#define PRINT_TABS_NL(x, y, z, ...)		make_text_with_tabs(x, z, y, __VA_ARGS__).print(); make_text(EMPTY_NEW_LINE).print()
#define PRINT_NL						make_text(EMPTY_NEW_LINE).print()
#define PRINT_EX(...)					render_text_ex(true, 0, __VA_ARGS__)
#define PRINT_EX_NNL(...)				render_text_ex(false, 0, __VA_ARGS__)
#define PRINT_INSTRUCTION(x, ...)		render_text_ex(true, x, __VA_ARGS__)
#define PRINT_INSTRUCTION_NNL(x, ...)	render_text_ex(false, x, __VA_ARGS__)

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