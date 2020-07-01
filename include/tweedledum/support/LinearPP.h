/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <ostream>
#include <valarray>

namespace tweedledum {

// Linear Phase Polynomial (LinerPP)
template<typename Parity = uint32_t>
class LinearPP {
	using Angle = double;
	using LinearTerm = std::pair<Parity, Angle>;

public:
	uint32_t size() const
	{
		return terms_.size();
	}

	auto begin() const
	{
		return terms_.cbegin();
	}

	auto end() const
	{
		return terms_.cend();
	}

	void add_term(Parity const& parity, Angle const& angle)
	{
		auto it = lower_bound(terms_.begin(), terms_.end(), parity);
		if (it != terms_.end() && it->first == parity) {
			it->second += angle;
			return;
		}
		terms_.emplace(it, parity, angle);
	}

	Angle extract_term(Parity const& parity)
	{
		auto it = lower_bound(terms_.begin(), terms_.end(), parity);
		if (it == terms_.end() || it->first != parity) {
			return 0;
		}
		Angle const angle = it->second;
		terms_.erase(it);
		return angle;
	}

private:
	using Iterator = typename std::vector<LinearTerm>::iterator;
	using DistType = typename std::iterator_traits<Iterator>::difference_type;

	Iterator lower_bound(Iterator first, Iterator last, Parity const& parity)
	{
		DistType len = std::distance(first, last);
		while (len > 0) {
			if (first->first == parity) {
				break;
			}
			DistType half = len >> 1;
			Iterator middle = first;
			std::advance(middle, half);
			if (middle->first < parity) {
				first = ++middle;
				len -= half + 1;
			} else {
				len = half;
			}
		}
		return first;
	}

	std::vector<LinearTerm> terms_;
};

} // namespace tweedledum
