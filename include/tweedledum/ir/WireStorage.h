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

	Wire find_wire(std::string_view name) const
	{
		for (WireInfo const& e : wires_) {
			if (e.name == name) {
				return e.wire;
			}
		}
		return Wire::invalid();
	}

protected:
	Wire do_create_qubit(std::string_view name)
	{
		Wire qubit = Wire::qubit(wires_.size());
		wires_.emplace_back(qubit, name);
		++num_qubits_;
		return qubit;
	}

	Wire do_create_cbit(std::string_view name)
	{
		Wire cbit = Wire::cbit(wires_.size());
		wires_.emplace_back(cbit, name);
		return cbit;
	}

private:
	struct WireInfo {
		Wire wire;
		std::string name;

		WireInfo(Wire const wire, std::string_view name)
		    : wire(wire), name(name)
		{}
	};

	uint32_t num_qubits_ = 0u;
	std::vector<WireInfo> wires_;
};

} // namespace tweedledum
