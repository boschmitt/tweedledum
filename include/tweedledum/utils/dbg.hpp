/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include <chrono>
#include <csignal>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <sys/types.h>
#include <type_traits>
#include <vector>

// Took some ideas from: https://github.com/sharkdp/dbg-macro

namespace tweedledum {

#ifdef TWEEDLEDUM_DEBUGGER
#pragma message("WARNING: using the dbg!")

namespace detail {

// https://blog.galowicz.de/2016/02/20/short_file_macro/
static constexpr char const* past_last_slash(char const* str, char const* last_slash)
{
	return *str == '\0' ? last_slash :
	                      *str == '/' ? past_last_slash(str + 1, str + 1) :
	                                    past_last_slash(str + 1, last_slash);
}

static constexpr char const* past_last_slash(char const* str)
{
	return past_last_slash(str, str);
}

template <typename T>
struct type_tag {};

// Adapted from: https://stackoverflow.com/a/35943472
template<class T>
constexpr std::string_view constexrp_type_name(type_tag<T>)
{
	char const* begin_ptr = __PRETTY_FUNCTION__;
	while (*begin_ptr++ != '=');
	for (; *begin_ptr == ' '; ++begin_ptr);

	char const* end_ptr = begin_ptr;
	uint32_t count_brakets = 1u;
	for (;; ++end_ptr) {
		switch (*end_ptr) {
		case '[':
			++count_brakets;
			break;

		case ']':
			--count_brakets;
			if (count_brakets == 0) {
				return {begin_ptr, std::size_t(end_ptr - begin_ptr)};
			}
			break;
		}
	}
	return {};
}

template <typename T>
std::string type_name(type_tag<T>) {
    constexpr auto type_name_ = constexrp_type_name(type_tag<T>());
	return std::string(type_name_);
}

template<typename T>
std::string type_name(type_tag<std::vector<T, std::allocator<T>>>)
{
	constexpr auto type_name_ = constexrp_type_name(type_tag<T>());
	return "std::vector<" + std::string(type_name_) + ">";
}

template<typename T>
std::string type_name_()
{
	if (std::is_volatile<T>::value) {
		if (std::is_pointer<T>::value) {
			return type_name_<typename std::remove_volatile<T>::type>() + " volatile";
		} else {
			return "volatile " + type_name_<typename std::remove_volatile<T>::type>();
		}
	}
	if (std::is_const<T>::value) {
		if (std::is_pointer<T>::value) {
			return type_name_<typename std::remove_const<T>::type>() + " const";
		} else {
			return "const " + type_name_<typename std::remove_const<T>::type>();
		}
	}
	if (std::is_pointer<T>::value) {
		return type_name_<typename std::remove_pointer<T>::type>() + "*";
	}
	if (std::is_lvalue_reference<T>::value) {
		return type_name_<typename std::remove_reference<T>::type>() + "&";
	}
	if (std::is_rvalue_reference<T>::value) {
		return type_name_<typename std::remove_reference<T>::type>() + "&&";
	}
	return type_name(type_tag<T>());
}

template<typename T, typename _ = void>
struct is_container : std::false_type {};

template<typename T>
struct is_container<
    T, std::void_t<typename T::value_type, typename T::size_type, typename T::allocator_type,
                   typename T::iterator, typename T::const_iterator, decltype(std::declval<T>().size()),
                   decltype(std::declval<T>().begin()), decltype(std::declval<T>().end()),
                   decltype(std::declval<T>().cbegin()), decltype(std::declval<T>().cend())>>
    : public std::true_type {};

template<class T>
inline constexpr bool is_container_v = is_container<T>::value;

template<typename T>
T&& identity(T&& t)
{
	return std::forward<T>(t);
}

} // namespace detail

class dbg {
	using steady_clock_t = std::chrono::steady_clock;
	using steady_time_t = steady_clock_t::time_point;

public:
	class indent_ {
	public:
		indent_(int& i)
		    : current_indent(i)
		{
			++current_indent;
		}

		~indent_()
		{
			--current_indent;
		}

	private:
		int& current_indent;
	};

public:
	dbg(char const* file_name, int line, char const* function_name)
	    : start_time_(steady_clock_t::now())
	{
		if (indent_level_ > 0) {
			fmt::print("{: >7}{: >{}}", "", "", 4 * indent_level_);
		}
		fmt::print(fg(fmt::color::yellow), "[{}:{}] {}\n", file_name, line, function_name);
	}

	~dbg()
	{
		steady_time_t end_time = steady_clock_t::now();
		steady_clock_t::duration duration = end_time - start_time_;
		if (indent_level_ > 0) {
			fmt::print("{: >7}{: >{}}", "", "", 4 * indent_level_);
		}
		fmt::print(fg(fmt::color::yellow), "Duration: {}\n", duration);
	}

	indent_ indent()
	{
		return indent_(indent_level_);
	}

	template<typename T>
	T&& value(int line, char const* expr, std::string const& type, T&& value) const
	{
		fmt::print(fg(fmt::color::gray), "[{: >4}]", line);
		fmt::print("{: >{}} ", "", 4 * indent_level_);
		fmt::print(fg(fmt::color::cyan), "{}", expr);
		T const& ref = value;
		print(ref);
		fmt::print(fg(fmt::color::light_green), "{}", type);
		fmt::print(")\n");
		return std::forward<T>(value);
	}

	template<typename... Args>
	void message(int line, Args&&... args)
	{
		fmt::print(fg(fmt::color::gray), "[{: >4}]", line);
		fmt::print("{: >{}} ", "", 4 * indent_level_);
		fmt::print("{}\n", fmt::format(std::forward<Args>(args)...));
	}

private:
	template<typename T>
	inline void print(T const& value) const
	{
		if constexpr (detail::is_container_v<T>) {
			const size_t size = value.size();
			const size_t n = std::min(size_t(10), size);
			fmt::print(" = {{ {}{} }} (",
			           fmt::join(value.begin(), value.begin() + n, ", "),
			           size > n ? ", ..." : "");
		} else {
			fmt::print(" = {} (", value);
		}
	}

private:
	static int indent_level_;
	steady_time_t start_time_;
};

int dbg::indent_level_ = -1;

#define __SHORT_FILE__ ({constexpr char const* sf__ {detail::past_last_slash(__FILE__)}; sf__;})

#define dbg_pick(expr) dbg_##__func__.value(__LINE__, #expr, detail::type_name_<decltype(expr)>(), expr)

#define dbg_msg(...) dbg_##__func__.message(__LINE__, __VA_ARGS__)

#define dbg_indent(...) auto dbg_indent_##__func__ = dbg_##__func__.indent()

#define dbg_start() dbg dbg_##__func__(__SHORT_FILE__, __LINE__, __func__); auto dbg_indent_##__func__ = dbg_##__func__.indent()

#undef TWEEDLEDUM_DEBUGGER

#else

#define dbg_pick(expr) detai::identity(expr)

#define dbg_msg(...)

#define dbg_indent(...)

#define dbg_start()

#endif

} // namespace tweedledum
