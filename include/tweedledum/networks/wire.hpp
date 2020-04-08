/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include <cassert>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace tweedledum::wire {

/* \brief Simple class to hold a wire identifier ``wire::id``
 *
 *  In tweedledum, a wire can be either a quantum or classical.  A quantum wire holds the state of a
 *  qubit, and it is represented by a line in quantum circuit diagrams.  In tweedledum, a quantum
 *  wire is equivalent to a qubit.  Similarly, a classical wire holds the state of a cbit, and it
 *  is represented by a double line in quantum circuit diagrams.
 *
 *  An ``wire::id`` serves three purposes:
 *  1) Uniquely identifying a wire within a circuit
 *  2) Informing if a wire is a qubit or a cbit
 *  3) When used withing controlled gates, informing if the use of the wire is complemented or not
 *     e.g., a CNOT can be negatively controlled by adding a NOT before and after the control
 *
 * Limits: a circuit can only have 2^30 - 1 wires! (the last one I use to identify a invalid wire)
 */
class id {
public:
#pragma region Types and constructors
	constexpr id(uint32_t const uid, bool const is_qubit)
	    : uid_(uid)
	    , is_qubit_(static_cast<uint32_t>(is_qubit))
	    , is_complemented_(0)
	{
		assert(uid <= (std::numeric_limits<uint32_t>::max() >> 2));
	}

	constexpr id(uint32_t const uid, bool const is_qubit, bool const is_complemented)
	    : uid_(uid)
	    , is_qubit_(static_cast<uint32_t>(is_qubit))
	    , is_complemented_(static_cast<uint32_t>(is_complemented))
	{
		assert(uid <= (std::numeric_limits<uint32_t>::max() >> 2));
	}
#pragma endregion

#pragma region Properties
	uint32_t uid() const
	{
		return uid_;
	}

	bool is_complemented() const
	{
		return static_cast<bool>(is_complemented_);
	}

	bool is_qubit() const
	{
		return static_cast<bool>(is_qubit_);
	}

	/*! \brief Guarantee the return of an uncomplemented ``wire::id`` */
	id wire() const
	{
		return id(uid(), is_qubit());
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
		return uid_;
	}

	id operator!() const
	{
		id complemented(*this);
		complemented.complement();
		return complemented;
	}

	bool operator<(id other) const
	{
		return data_ < other.data_;
	}

	bool operator==(id other) const
	{
		return data_ == other.data_;
	}

	bool operator!=(id other) const
	{
		return data_ != other.data_;
	}
#pragma endregion

private:
	union {
		uint32_t data_;
		struct {
			uint32_t uid_ : 30;
			uint32_t is_qubit_ : 1;
			uint32_t is_complemented_ : 1;
		};
	};
};

constexpr id make_qubit(uint32_t const uid, bool is_complemented = false)
{
	return id(uid, /* is_qubit */ true, is_complemented);
}

constexpr id make_cbit(uint32_t const uid, bool is_complemented = false)
{
	return id(uid, /* is_qubit */ false, is_complemented);
}

constexpr id invalid_id = id(std::numeric_limits<uint32_t>::max() >> 2, true, true);

enum class modes : uint8_t {
	in,
	out,
	inout,
	ancilla,
};

/* \brief Class used for storing wire information in a circuit
 */
class storage {
	struct wire_info {
		id wire_id;
		modes mode;
		std::string name;

		wire_info(id const wire_id, modes const mode, std::string_view name)
		    : wire_id(wire_id)
		    , mode(mode)
		    , name(name)
		{}
	};

public:
	storage()
	    : num_qubits_(0u)
	{}

	uint32_t num_wires() const
	{
		return wires_.size();
	}

	uint32_t num_qubits() const
	{
		return num_qubits_;
	}

	uint32_t num_cbits() const
	{
		return num_wires() - num_qubits();
	}

	id create_qubit(std::string_view name, modes const mode)
	{
		id const qubit_id = make_qubit(wires_.size());
		wires_.emplace_back(qubit_id, mode, name);
		name_to_wire_.emplace(name, qubit_id);
		++num_qubits_;
		return qubit_id;
	}

	id create_cbit(std::string_view name, modes const mode)
	{
		id const cbit_id = make_cbit(wires_.size());
		wires_.emplace_back(cbit_id, mode, name);
		name_to_wire_.emplace(name, cbit_id);
		return cbit_id;
	}

	id wire(std::string_view name) const
	{
		return name_to_wire_.at(std::string(name));
	}

	std::string wire_name(id const wire_id) const
	{
		return wires_.at(wire_id).name;
	}

	/* \brief Add a new name to identify a wire.
	 *
	 * \param rename If true, this flag indicates that `new_name` must substitute the previous
	 *               name. (default: `true`)
	 */
	void wire_name(id const wire_id, std::string_view new_name, bool const rename)
	{
		if (rename) {
			name_to_wire_.erase(wires_.at(wire_id).name);
			wires_.at(wire_id).name = new_name;
		}
		name_to_wire_.emplace(new_name, wire_id);
	}

	modes wire_mode(id const wire_id) const
	{
		return wires_.at(wire_id).mode;
	}

	void wire_mode(id const wire_id, modes const new_mode)
	{
		wires_.at(wire_id).mode = new_mode;
	}

	template<typename Fn>
	void foreach_wire(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, id> ||
			      std::is_invocable_r_v<void, Fn, id, std::string_view>);
		// clang-format on
		for (uint32_t i = 0u; i < wires_.size(); ++i) {
			if constexpr (std::is_invocable_r_v<void, Fn, id>) {
				fn(wires_.at(i).wire_id);
			} else {
				fn(wires_.at(i).wire_id, wires_.at(i).name);
			}
		}
	}

private:
	uint32_t num_qubits_ = 0u;
	std::vector<wire_info> wires_;
	std::unordered_map<std::string, id> name_to_wire_;
};

} // namespace tweedledum::wire
