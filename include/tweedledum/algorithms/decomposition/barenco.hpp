/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate.hpp"
#include "../../networks/wire.hpp"
#include "../../utils/angle.hpp"
#include "../utility/shallow_duplicate.hpp"

#include <cstdint>
#include <vector>

namespace tweedledum {

/*! \brief Parameters for `barenco_decomposition`. */
struct barenco_params {
	uint32_t controls_threshold = 2u;
	bool use_ncrx = true; // Relative phase
};

namespace detail {

// Barenco, A., Bennett, C.H., Cleve, R., DiVincenzo, D.P., Margolus, N., Shor, P., Sleator, T., Smolin,
// J.A. and Weinfurter, H., 1995. Elementary gates for quantum computation. Physical review A, 52(5), p.3457.
template<class Circuit>
void barenco_decomp(Circuit& circuit, gate const& g, std::vector<wire::id> const& controls,
                    wire::id target, barenco_params const& params)
{
	assert(params.controls_threshold >= 2);
	uint32_t const num_controls = controls.size();
	assert(num_controls >= 2);

	if (num_controls <= params.controls_threshold) {
		circuit.create_op(g, controls, std::vector<wire::id>({target}));
		return;
	}

	std::vector<wire::id> workspace;
	circuit.foreach_wire([&](wire::id wire) {
		if (!wire.is_qubit()) {
			return;
		}
		if (wire == target) {
			return;
		}
		for (wire::id control : controls) {
			if (wire.uid() == control.uid()) {
				return;
			}
		}
		workspace.push_back(wire);
	});
	const uint32_t workspace_size = workspace.size();
	if (workspace_size == 0) {
		std::cout << "[e] no sufficient helper line found for mapping, break\n";
		return;
	}

	gate const& compute_gate = params.use_ncrx ? gate_lib::ncrx(sym_angle::pi) : gate_lib::ncx;
	gate const& uncompute_gate = params.use_ncrx ? gate_lib::ncrx(-sym_angle::pi) : gate_lib::ncx;
	// Check if there are enough empty lines lines
	// Lemma 7.2: If n ≥ 5 and m ∈ {3, ..., ⌈n/2⌉} then a gate can be simulated by a circuit
	// consisting of 4(m − 2) gates.
	// n is the number of qubits
	// m is the number of controls
	if (circuit.num_qubits() + 1 >= (num_controls << 1)) {
		workspace.push_back(target);

		// When offset is equal to 0 this is computing the toffoli
		// When offset is 1 this is cleaning up the workspace, that is, restoring the state
		// to their initial state
		for (int offset = 0; offset <= 1; ++offset) {
			for (int i = offset; i < static_cast<int>(num_controls) - 2; ++i) {
				circuit.create_op(i ? compute_gate : gate_lib::ncx,
				                  std::vector({controls[num_controls - 1 - i],
				                               workspace[workspace_size - 1 - i]}),
				                  std::vector({workspace[workspace_size - i]}));
			}

			circuit.create_op(offset ? uncompute_gate : compute_gate,
			                  std::vector({controls[0], controls[1]}),
			                  std::vector({workspace[workspace_size - (num_controls - 2)]}));

			for (int i = num_controls - 2 - 1; i >= offset; --i) {
				circuit.create_op(i ? uncompute_gate : gate_lib::ncx,
				                  std::vector({controls[num_controls - 1 - i],
				                               workspace[workspace_size - 1 - i]}),
				                  std::vector({workspace[workspace_size - i]}));
			}
		}
		return;
	}

	// Not enough qubits in the workspace, extra decomposition step
	// Lemma 7.3: For any n ≥ 5, and m ∈ {2, ... , n − 3} a (n−2)-toffoli gate can be simulated
	// by a circuit consisting of two m-toffoli gates and two (n−m−1)-toffoli gates
	std::vector<wire::id> controls0;
	std::vector<wire::id> controls1;
	for (auto i = 0u; i < (num_controls >> 1); ++i) {
		controls0.push_back(controls[i]);
	}
	for (auto i = (num_controls >> 1); i < num_controls; ++i) {
		controls1.push_back(controls[i]);
	}
	wire::id free_qubit = workspace.front();
	controls1.push_back(free_qubit);
	barenco_decomp(circuit, compute_gate, controls0, free_qubit, params);
	barenco_decomp(circuit, g, controls1, target, params);
	barenco_decomp(circuit, uncompute_gate, controls0, free_qubit, params);
	barenco_decomp(circuit, g, controls1, target, params);
}

} /* namespace detail */

/*! \brief Barenco decomposition.
 *
 * Decomposes all n-controlled gates with more than ``controls_threshold`` controls into gates with
 * at most ``controls_threshold`` controls. This may introduce one additional helper qubit called
 * ancilla.
 *
 */
template<typename Circuit>
Circuit barenco_decomposition(Circuit const& circuit, barenco_params params = {})
{
	using op_type = typename Circuit::op_type;
	Circuit result = shallow_duplicate(circuit);
	circuit.foreach_op([&](op_type const& op) {
		if (op.is_one_qubit()) {
			result.create_op(op, op.target());
		} else if (op.is_two_qubit()) {
			result.create_op(op, op.control(), op.target());
		} else {
			std::vector<wire::id> controls;
			std::vector<wire::id> targets;
			op.foreach_control([&](wire::id control) {
				controls.push_back(control);
			});
			op.foreach_target([&](wire::id target) {
				targets.push_back(target);
			});
			detail::barenco_decomp(result, op, controls, targets.at(0), params);
		}
	});
	return result;
}

} // namespace tweedledum
