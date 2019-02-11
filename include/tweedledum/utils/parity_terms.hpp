/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "angle.hpp"

#include <cassert>
#include <fmt/format.h>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace tweedledum {

/*! \brief
 */
class parity_terms {
public:
#pragma region Types and constructors
	parity_terms()
	{}
#pragma endregion

#pragma region Properties
	auto num_terms() const
	{
		return term_to_angle_.size();
	}
#pragma endregion

#pragma region Iterators
	auto begin() const
	{
		return term_to_angle_.cbegin();
	}

	auto end() const
	{
		return term_to_angle_.cend();
	}
#pragma endregion

#pragma region Modifiers
	/*! \brief Add parity term.
	 *
	 * If the term already exist it increments the rotation angle
	 */
	void add_term(uint32_t term, angle rotation_angle)
	{
		assert(rotation_angle != 0.0);
		auto search = term_to_angle_.find(term);
		if (search != term_to_angle_.end()) {
			search->second += rotation_angle;
		} else {
			term_to_angle_.emplace(term, rotation_angle);
		}
	}

	/*! \brief Extract parity term. */
	auto extract_term(uint32_t term)
	{
		auto node_handle = term_to_angle_.extract(term);
		if (node_handle.empty()) {
			return angle(0.0);
		}
		return node_handle.mapped();
	}
#pragma endregion

private:
	std::unordered_map<uint32_t, angle> term_to_angle_;
};

} // namespace tweedledum
