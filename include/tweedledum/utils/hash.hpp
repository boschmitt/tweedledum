/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include <functional>
#include <set>

namespace tweedledum {

template<typename T>
struct hash : public std::hash<T> {};

template<>
struct hash<std::set<uint32_t>> {
	using argument_type = std::set<uint32_t>;
	using result_type = size_t;

	void combine(std::size_t& seed, uint32_t const& v) const
	{
		seed ^= std::hash<uint32_t>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	result_type operator()(argument_type const& in) const
	{
		result_type seed = 0;
		for (auto& element : in) {
			combine(seed, element);
		}
		return seed;
	}
};

} // namespace tweedledum
