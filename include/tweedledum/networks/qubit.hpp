/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include <cstdint>
#include <limits>

namespace tweedledum {

/* \brief Simple class to hold a qubit indentifier ``qid`` */
class qubit_id {
public:

#pragma region Types and constructors
	explicit qubit_id() = default;

	constexpr qubit_id(uint32_t index)
	    : literal_(index << 1)
	{}

	explicit qubit_id(uint32_t index, bool complemented)
	    : literal_((index << 1) | (complemented ? 1 : 0))
	{}
#pragma endregion

#pragma region Properties
	auto index() const
	{
		return literal_ >> 1;
	}

	auto is_complemented() const
	{
		return (literal_ & 1) == 1;
	}

	auto literal() const
	{
		return literal_;
	}
#pragma endregion
	
#pragma region Modifiers
	void complement()
	{
		literal_ ^= 1;
	}
#pragma endregion

#pragma region Overloads
	operator uint32_t() const
	{
		return (literal_ >> 1);
	}

	qubit_id operator!() const
	{
		qubit_id c_;
		c_.literal_ = literal_ ^ 1;
		return c_;
	}

	bool operator<(qubit_id other) const
	{
		return literal_ < other.literal_;
	}

	bool operator==(qubit_id other) const
	{
		return literal_ == other.literal_;
	}

	bool operator!=(qubit_id other) const
	{
		return literal_ != other.literal_;
	}
#pragma endregion

private:
	uint32_t literal_ = std::numeric_limits<uint32_t>::max();
};

constexpr auto qid_invalid = qubit_id(std::numeric_limits<uint32_t>::max());

} // namespace tweedledum
