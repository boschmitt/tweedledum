/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "Wire.h"

#include <vector>

namespace tweedledum {

class WireStorage {
public:
	WireStorage() : num_qubits_(0u) {}

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

	auto begin_wire() const
	{
		return wires_.cbegin();
	}

	auto end_wire() const
	{
		return wires_.cend();
	}

protected:
	WireRef do_create_qubit(std::string_view name)
	{
		uint32_t uid = wires_.size();
		wires_.emplace_back(uid, name, Wire::Kind::quantum);
		num_qubits_++;
		return {uid, Wire::Kind::quantum};
	}

	WireRef do_create_cbit(std::string_view name)
	{
		uint32_t uid = wires_.size();
		wires_.emplace_back(uid, name, Wire::Kind::classical);
		return {uid, Wire::Kind::classical};
	}

private:
	uint32_t num_qubits_ = 0u;
	std::vector<Wire> wires_;
};

} // namespace tweedledum
