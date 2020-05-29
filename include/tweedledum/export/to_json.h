/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../ir/Circuit.h"

#include <algorithm>
#include <nlohmann/json.hpp>

namespace tweedledum {

// TODO: ideally I would encode this "control_state" into the qubits ids
//       they basically indicate whether a controlled operantion is controlled
//       on the positive or negative polarity of a given qubit.
inline void to_json(nlohmann::json& j, Instruction const& inst)
{
	std::vector<uint32_t> controls;
	std::string control_state;
	std::for_each(inst.begin(), inst.end() - 1, [&](WireRef const& wire) {
		controls.push_back(wire.uid());
		control_state.push_back(
		    wire.polarity() == WireRef::Polarity::positive ? '1' : '0');
	});
	j = nlohmann::json{{"gate", inst.kind()},
	    {"qubits", {inst.target().uid()}},
	    {"control_qubits", controls}, {"control_state", control_state}};
}

inline void to_json(nlohmann::json& j, Circuit const& circuit)
{
	j = nlohmann::json{{"num_qubits", circuit.num_qubits()},
	    {"gates", circuit.instruction_}};
}

} // namespace tweedledum
