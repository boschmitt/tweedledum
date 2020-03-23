/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include <cassert>
#include <cstdint>
#include <limits>

namespace tweedledum {

/* \brief Simple class to hold a wire identifier ``wire_id``
 *
 *  In tweedledum, A wire can be either a quantum or classical. A quantum wire holds the state of a
 *  qubit, and it is represented by a line in quantum circuit diagrams. In tweedledum, a quantum
 *  wire is equivalent to a qubit. Similarly, a classical wire holds the state of a cbit, and it
 *  is represented by a double line in quantum circuit diagrams.
 *
 *  An ``wire_id`` serves three purposes:
 *  1) Uniquely identifying a wire within a network
 *  2) Informing if a wire is a qubit or a cbit
 *  3) When used withing controlled gates, informing if the use of the wire is complemented or not
 *     e.g., a CNOT can be negatively controlled by adding a NOT before and after the control
 *
 * Limits: a network can only have 2^30 - 1 wires! (the last one I use to identify a invalid wire)
 */
class wire_id {
public:
#pragma region Types and constructors
	constexpr wire_id(uint32_t index, bool is_qubit)
	    : id_(index)
	    , is_qubit_(static_cast<uint32_t>(is_qubit))
	    , is_complemented_(0)
	{
		assert(index <= (std::numeric_limits<uint32_t>::max() >> 2));
	}

	constexpr wire_id(uint32_t index, bool is_qubit, bool is_complemented)
	    : id_(index)
	    , is_qubit_(static_cast<uint32_t>(is_qubit))
	    , is_complemented_(static_cast<uint32_t>(is_complemented))
	{
		assert(index <= (std::numeric_limits<uint32_t>::max() >> 2));
	}
#pragma endregion

#pragma region Properties
	uint32_t id() const
	{
		return id_;
	}

	bool is_complemented() const
	{
		return static_cast<bool>(is_complemented_);
	}

	bool is_qubit() const
	{
		return static_cast<bool>(is_qubit_);
	}

	/*! \brief Guarantee the return of an uncomplemented ``wire_id`` */
	wire_id wire() const
	{
		return wire_id(id(), is_qubit());
	}
#pragma endregion

#pragma region Modifiers
	void complement()
	{
		is_complemented_ ^= 1u;
	}
#pragma endregion

#pragma region Overloads
	operator uint32_t() const
	{
		return id();
	}

	wire_id operator!() const
	{
		wire_id complemented(*this);
		complemented.complement();
		return complemented;
	}

	bool operator<(wire_id other) const
	{
		return data_ < other.data_;
	}

	bool operator==(wire_id other) const
	{
		return data_ == other.data_;
	}

	bool operator!=(wire_id other) const
	{
		return data_ != other.data_;
	}
#pragma endregion

private:
	union {
		uint32_t data_;
		struct {
			uint32_t id_ : 30;
			uint32_t is_qubit_ : 1;
			uint32_t is_complemented_ : 1;
		};
	};
};

namespace wire {
constexpr auto invalid = wire_id(std::numeric_limits<uint32_t>::max() >> 2, true, true);
} // namespace wire

enum class wire_modes : uint8_t {
	in,
	out,
	inout,
	ancilla,
};

} // namespace tweedledum
