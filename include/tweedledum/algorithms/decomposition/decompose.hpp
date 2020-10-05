/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate.hpp"
#include "../../networks/wire.hpp"
#include "../../support/angle.hpp"
#include "../synthesis/diagonal_synth.hpp"
#include "../utility/shallow_duplicate.hpp"
#include "barenco.hpp"
#include "gates/database.hpp"

#include <array>
#include <iostream>
#include <vector>

namespace tweedledum {

/*! \brief Parameters for `decomposition`. */
struct decomp_params {
	uint64_t gate_set = gate_set::clifford_t;
	uint32_t barenco_controls_threshold = 6u;
	bool allow_ancilla = true;
	bool use_t_par = false;
	bool use_relative_phase = false;
};

#pragma region Decomposition circuit builder (detail)
namespace detail {

template<typename Circuit>
class decomp_builder : public Circuit {
public:
	using op_type = typename Circuit::op_type;
	using node_type = typename Circuit::node_type;
	using dstrg_type = typename Circuit::dstrg_type;

	explicit decomp_builder(Circuit& circuit, decomp_params const& params)
	    : Circuit(circuit)
	    , params_(params)
	{
		barenco_params_.controls_threshold = params_.barenco_controls_threshold;
		barenco_params_.use_ncrx = params_.use_relative_phase;
	}

public:
#pragma region Creating operations (using wire ids)
	void create_op(gate const& g, wire::id const t)
	{
		if (params_.gate_set & (1ull << static_cast<uint32_t>(g.id()))) {
			this->emplace_op(op_type(g, t));
			return;
		}
		switch (g.id()) {
		// Non-parameterisable gates
		case gate_ids::i:
			create_op(gate_lib::u3(sym_angle::zero, sym_angle::zero, sym_angle::zero), t);
			return;

		case gate_ids::h:
			create_op(gate_lib::u3(sym_angle::pi_half, sym_angle::zero, sym_angle::pi), t);
			return;

		case gate_ids::x:
			create_op(gate_lib::u3(sym_angle::pi, sym_angle::zero, sym_angle::pi), t);
			return;

		case gate_ids::y:
			create_op(gate_lib::u3(sym_angle::pi, sym_angle::pi_half, sym_angle::pi_half), t);
			return;

		case gate_ids::z:
			create_op(gate_lib::r1(sym_angle::pi), t);
			return;

		case gate_ids::s:
			create_op(gate_lib::r1(sym_angle::pi_half), t);
			return;

		case gate_ids::sdg:
			create_op(gate_lib::r1(-sym_angle::pi_half), t);
			return;

		case gate_ids::t:
			create_op(gate_lib::r1(sym_angle::pi_quarter), t);
			return;

		case gate_ids::tdg:
			create_op(gate_lib::r1(-sym_angle::pi_quarter), t);
			return;
		
		// Parameterisable gates
		case gate_ids::r1:
			create_op(gate_lib::u3(sym_angle::zero, sym_angle::zero, g.rotation_angle()), t);
			return;

		case gate_ids::rx:
			create_op(gate_lib::u3(g.rotation_angle(), -sym_angle::pi_half, sym_angle::pi_half), t);
			return;

		case gate_ids::ry:
			create_op(gate_lib::u3(g.rotation_angle(), sym_angle::zero, sym_angle::zero), t);
			return;

		default:
			break;
		}
		this->emplace_op(op_type(g, t));
	}

	void create_op(gate const& g, wire::id const w0, wire::id const w1)
	{
		if (params_.gate_set & (1ull << static_cast<uint32_t>(g.id()))) {
			this->emplace_op(op_type(g, w0, w1));
			return;
		}
		switch (g.id()) {
		// Non-parameterisable gates
		case gate_ids::cx:
			if (params_.gate_set & (1ull << static_cast<uint32_t>(gate_ids::cz))) {
				create_op(gate_lib::h, w1);
				create_op(gate_lib::cz, w0, w1);
				create_op(gate_lib::h, w1);
				return;
			}
			break;

		case gate_ids::cy:
			create_op(gate_lib::sdg, w1);
			create_op(gate_lib::cx, w0, w1);
			create_op(gate_lib::s, w1);
			return;

		case gate_ids::cz:
			create_op(gate_lib::h, w1);
			create_op(gate_lib::cx, w0, w1);
			create_op(gate_lib::h, w1);
			return;

		case gate_ids::swap:
			create_op(gate_lib::cx, w0, w1);
			create_op(gate_lib::cx, w1, w0);
			create_op(gate_lib::cx, w0, w1);
			return;

		// Parameterisable gates
		case gate_ids::crx:
			create_op(gate_lib::r1(g.rotation_angle() / 2), w1);
			create_op(gate_lib::cx, w0, w1);
			create_op(gate_lib::ry(-g.rotation_angle() / 2), w1);
			create_op(gate_lib::cx, w0, w1);
			create_op(gate_lib::u3(g.rotation_angle() / 2, -sym_angle::pi_half,
			                       sym_angle::zero),
			          w1);
			return;

		case gate_ids::cry:
			create_op(gate_lib::ry(g.rotation_angle() / 2), w1);
			create_op(gate_lib::cx, w0, w1);
			create_op(gate_lib::ry(-g.rotation_angle() / 2), w1);
			create_op(gate_lib::cx, w0, w1);
			return;

		case gate_ids::crz:
			create_op(gate_lib::r1(g.rotation_angle() / 2), w1);
			create_op(gate_lib::cx, w0, w1);
			create_op(gate_lib::r1(-g.rotation_angle() / 2), w1);
			create_op(gate_lib::cx, w0, w1);
			return;

		default:
			break;
		}
		this->emplace_op(op_type(g, w0, w1));
	}

	void create_op(gate const& g, wire::id const c0, wire::id const c1, wire::id const t)
	{
		if (params_.gate_set & (1ull << static_cast<uint32_t>(g.id()))) {
			this->emplace_op(op_type(g, c0, c1, t));
			return;
		}
		switch (g.id()) {
		// Non-parameterisable gates
		case gate_ids::ncx:
			create_op(gate_lib::h, t);
			if (params_.use_t_par) {
				ccz_tpar(*this, c0, c1, t);
			} else {
				ccz(*this, c0, c1, t);
			}
			create_op(gate_lib::h, t);
			return;

		case gate_ids::ncy:
			create_op(gate_lib::sdg, t);
			create_op(gate_lib::h, t);
			if (params_.use_t_par) {
				ccz_tpar(*this, c0, c1, t);
			} else {
				ccz(*this, c0, c1, t);
			}
			create_op(gate_lib::h, t);
			create_op(gate_lib::s, t);
			break;

		case gate_ids::ncz:
			if (params_.use_t_par) {
				ccz_tpar(*this, c0, c1, t);
			} else {
				ccz(*this, c0, c1, t);
			}
			return;

		// Parameterisable gates
		case gate_ids::ncrx:
		case gate_ids::ncry:
			// create_op(gate_lib::ry(g.rotation_angle() / 2), w1);
			// create_op(gate_lib::ncx, c0, c1, t);
			// create_op(gate_lib::ry(-g.rotation_angle() / 2), w1);
			// create_op(gate_lib::ncx, c0, c1, t);
			break;

		case gate_ids::ncrz:
		default:
			break;
		}
		this->emplace_op(op_type(g, c0, c1, t));
	}

	void create_op(gate const& g, std::vector<wire::id> const& controls,
	               std::vector<wire::id> const& targets)
	{
		if (params_.gate_set & (1ull << static_cast<uint32_t>(g.id()))) {
			if (controls.size() <= params_.barenco_controls_threshold) {
				this->emplace_op(op_type(g, controls, targets));
				return;
			}
		}

		if (controls.size() == 2u) {
			create_op(g, controls.at(0), controls.at(1), targets.at(0));
			return;
		} else if (controls.size() > params_.barenco_controls_threshold) {
			barenco_create_op(g, controls, targets.at(0));
			return;
		} else if (controls.size() == 3u && g.is(gate_ids::ncx)) {
			if (controls.size() + 1u == this->num_qubits()) {
				this->create_qubit(wire::modes::ancilla);
			}
			cccx(*this, controls, targets.at(0));
			return;
		} else if (controls.size() == 4u && g.is(gate_ids::ncx)) {
			if (controls.size() + 1u == this->num_qubits()) {
				this->create_qubit(wire::modes::ancilla);
			}
			ccccx(*this, controls, targets.at(0));
			return;
		}
		diagonal_create_op(g, controls, targets.at(0));
	}
#pragma endregion

private:
	void barenco_create_op(gate const& g, std::vector<wire::id> const& controls, wire::id target)
	{
		if (controls.size() + 1u == this->num_qubits()) {
			this->create_qubit(wire::modes::ancilla);
		}
		if (params_.gate_set & (1ull << static_cast<uint32_t>(g.id()))) {
			barenco_decomp(*this, g, controls, target, barenco_params_);
			return;
		}
		switch (g.id()) {
		// Non-parameterisable gates
		case gate_ids::ncx:
			barenco_decomp(*this, g, controls, target, barenco_params_);
			return;

		case gate_ids::ncy:
			create_op(gate_lib::sdg, target);
			barenco_decomp(*this, gate_lib::ncx, controls, target, barenco_params_);
			create_op(gate_lib::s, target);
			break;

		case gate_ids::ncz: 
			create_op(gate_lib::h, target);
			barenco_decomp(*this, gate_lib::ncx, controls, target, barenco_params_);
			create_op(gate_lib::h, target);
			break;

		// Parameterisable gates
		case gate_ids::ncrx:
			break;

		case gate_ids::ncry:
			break;

		case gate_ids::ncrz:
			break;

		default:
			break;
		}
	}

	void diagonal_create_op(gate const& g, std::vector<wire::id> controls, wire::id target)
	{
		controls.emplace_back(target);
		std::vector<angle> angles((1 << controls.size()), sym_angle::zero);

		switch (g.id()) {
		// Non-parameterisable gates
		case gate_ids::ncx:
			angles.back() = sym_angle::pi;
			create_op(gate_lib::h, target);
			diagonal_synth(*this, controls, angles);
			create_op(gate_lib::h, target);
			return;

		case gate_ids::ncy:
			angles.back() = sym_angle::pi;
			create_op(gate_lib::sdg, target);
			create_op(gate_lib::h, target);
			diagonal_synth(*this, controls, angles);
			create_op(gate_lib::h, target);
			create_op(gate_lib::s, target);
			break;

		case gate_ids::ncz:
			angles.back() = sym_angle::pi;
			diagonal_synth(*this, controls, angles);
			break;

		// Parameterisable gates
		case gate_ids::ncrx:
			angles.at(angles.size() - 2u) = -g.rotation_angle();
			angles.at(angles.size() - 1u) = g.rotation_angle();
			create_op(gate_lib::h, target);
			diagonal_synth(*this, controls, angles);
			create_op(gate_lib::h, target);
			break;

		case gate_ids::ncry:
			break;

		case gate_ids::ncrz:
			angles.at(angles.size() - 2u) = -g.rotation_angle();
			angles.at(angles.size() - 1u) = g.rotation_angle();
			diagonal_synth(*this, controls, angles);
			break;

		default:
			break;
		}
	}

private:
	decomp_params params_;
	barenco_params barenco_params_;
};

} // namespace detail
#pragma endregion

/*! \brief 
 *
 * \tparam Circuit the circuit type.
 * \param[in] circuit the original quantum circuit (__will not be modified__).
 * \returns a decomposed circuit.
 */
template<typename Circuit>
Circuit decompose(Circuit const& circuit, decomp_params params = {})
{
	using op_type = typename Circuit::op_type;
	Circuit result = shallow_duplicate(circuit);
	detail::decomp_builder decomp(result, params);

	circuit.foreach_op([&](op_type const& op) {
		if (op.is_one_qubit()) {
			decomp.create_op(op, op.target());
		} else if (op.is_two_qubit()) {
			decomp.create_op(op, op.control(), op.target());
		} else {
			std::vector<wire::id> controls;
			std::vector<wire::id> targets;
			op.foreach_control([&](wire::id control) {
				controls.push_back(control);
			});
			op.foreach_target([&](wire::id target) {
				targets.push_back(target);
			});
			decomp.create_op(op, controls, targets);
		}
	});
	return result;
}

} // namespace tweedledum
