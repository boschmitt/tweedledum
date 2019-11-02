/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "token_kinds.hpp"

#include <cstdint>
#include <string>

namespace tweedledee {
namespace quil {

// The token class is used to represent a single lexed token.
struct token {
	token_kinds kind = token_kinds::unknown;
	uint32_t location = 0u;
	uint32_t length = 0u;
	const char* content_ptr = nullptr;

public:
	token() = default;

	token(token_kinds k, unsigned loc, unsigned len, const char* content)
	    : kind(k)
	    , location(loc)
	    , length(len)
	    , content_ptr(content)
	{}

	std::string_view name() const
	{
		return token_name(kind);
	}

	// uint32_t location() const
	// {
	// 	return location;
	// }

	bool is(token_kinds k) const
	{
		return kind == k;
	}

	bool is_not(token_kinds k) const
	{
		return kind != k;
	}

	template<typename... Ts>
	bool is_one_of(token_kinds k1, token_kinds k2, Ts... ks) const
	{
		return is(k1) || is_one_of(k2, ks...);
	}

	// TODO: check this further, feel like a really ugly hack.
	operator float()
	{
		return std::stof(std::string(content_ptr, length));
	}

	operator std::string_view()
	{
		return std::string_view(content_ptr, length);
	}

	operator std::uint32_t()
	{
		return std::stoi(std::string(content_ptr, length));
	}

	operator std::int32_t()
	{
		return std::stoi(std::string(content_ptr, length));
	}
};

} // namespace quil
} // namespace tweedledee
