/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../ir/Circuit.h"
#include "../../ir/GateLib.h"
#include "../../ir/Wire.h"
#include "../../support/LinearPP.h"
#include "all_linear_synth.h"
#include "gray_synth.h"

#include <cassert>
#include <vector>

namespace tweedledum {
#pragma region Implementation details
namespace diagonal_synth_detail {

inline void complement_qubit(uint32_t i, std::vector<double>& angles)
{
	uint32_t const size_2 = (angles.size() / 2);
	uint32_t const step = (size_2 >> i);
	for (uint32_t j = 0u; j < size_2; j += step) {
		for (uint32_t k = (j << 1); k < (step + (j << 1)); k += 1) {
			std::swap(angles.at(k), angles.at(step + k));
		}
	}
}

inline std::vector<double> fix_angles(
    std::vector<WireRef>& qubits, std::vector<double> const& angles)
{
	std::vector<double> new_angles;
	std::transform(angles.begin(), angles.end(),
	    std::back_inserter(new_angles), [](double a) {
		    return -a;
	    });

	// Normalize qubits polarity
	uint32_t index = 0u;
	for (uint32_t i = qubits.size(); i-- > 0;) {
		if (!qubits.at(index).is_complemented()) {
			index += 1;
			continue;
		}
		qubits.at(index).complement();
		complement_qubit(index, new_angles);
		index += 1;
	}
	return new_angles;
}

inline void fast_hadamard_transform(std::vector<double>& angles)
{
	uint32_t k = 0u;
	for (uint32_t m = 1u; m < angles.size(); m <<= 1u) {
		for (uint32_t i = 0u; i < angles.size(); i += (m << 1u)) {
			for (uint32_t j = i, p = k = i + m; j < p; ++j, ++k) {
				double t = angles.at(j);
				angles.at(j) += angles.at(k);
				angles.at(k) = t - angles.at(k);
			}
		}
	}
}

} // namespace diagonal_synth_detail
#pragma endregion

inline void diagonal_synth(Circuit& circuit, std::vector<WireRef> qubits,
    std::vector<double> const& angles)
{
	// Number of angles + 1 needs to be a power of two!
	assert(!angles.empty() && !(angles.size() & (angles.size() - 1)));
	assert(!qubits.empty() && qubits.size() <= 32);
	assert((1u << qubits.size()) == angles.size());

	std::vector<double> new_angles
	    = diagonal_synth_detail::fix_angles(qubits, angles);
	diagonal_synth_detail::fast_hadamard_transform(new_angles);
	LinearPP parities;
	uint32_t factor = (1 << (qubits.size() - 1));
	for (uint32_t i = 0u; i < new_angles.size(); ++i) {
		if (new_angles.at(i) == 0) {
			continue;
		}
		parities.add_term(i, new_angles.at(i) / factor);
	}
	if (parities.size() == new_angles.size()) {
		all_linear_synth(circuit, qubits, parities);
	} else {
		gray_synth(circuit, qubits,
		    Matrix<uint8_t>::Identity(qubits.size()), parities);
	}
}

inline Circuit diagonal_synth(std::vector<double> const& angles)
{
	// Number of angles + 1 needs to be a power of two!
	assert(!angles.empty() && !(angles.size() & (angles.size() - 1)));
	uint32_t num_qubits = __builtin_ctz(angles.size());
	assert(num_qubits <= 32u);

	// TODO: method to generate a name;
	Circuit circuit("my_circuit");
	// Create the necessary qubits
	std::vector<WireRef> wires;
	wires.reserve(num_qubits);
	for (uint32_t i = 0u; i < num_qubits; ++i) {
		wires.emplace_back(circuit.create_qubit());
	}
	diagonal_synth(circuit, wires, angles);
	return circuit;
}

} // namespace tweedledum