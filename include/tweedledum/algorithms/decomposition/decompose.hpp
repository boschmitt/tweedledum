/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate.hpp"
#include "../../networks/wire_id.hpp"
#include "../../utils/angle.hpp"
#include "../generic/rewrite.hpp"
#include "../synthesis/diagonal_synth.hpp"
#include "gates/database.hpp"
#include "barenco.hpp"

#include <array>
#include <iostream>
#include <vector>

namespace tweedledum {

/*! \brief Parameters for `decomposition`. */
struct decomp_params {
	uint64_t gate_set = gate_set::clifford_t;
	uint32_t controls_threshold = 2u;
	bool allow_ancilla = true;
	bool use_barenco = true;
	bool use_t_par = false;
	bool use_relative_phase = false;
};

#pragma region Decomposition network builder (detail)
namespace detail {

template<typename Network>
class decomp_builder : public Network {
public:
	using op_type = typename Network::op_type;
	using node_type = typename Network::node_type;
	using dstrg_type = typename Network::dstrg_type;

	explicit decomp_builder(Network& network, decomp_params const& params)
	    : Network(network)
	    , params_(params)
	{ }

public:
#pragma region Creating operations (using wire ids)
	void create_op(gate const& g, wire_id t)
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

	void create_op(gate const& g, wire_id w0, wire_id w1)
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

	void create_op(gate const& g, wire_id c0, wire_id c1, wire_id t)
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

	void create_op(gate const& g, std::vector<wire_id> const& controls,
	               std::vector<wire_id> const& targets)
	{
		if (params_.gate_set & (1ull << static_cast<uint32_t>(g.id()))) {
			if (controls.size() <= params_.controls_threshold) {
				this->emplace_op(op_type(g, controls, targets));
				return;
			}
		}

		if (controls.size() == 2u) {
			create_op(g, controls.at(0), controls.at(1), targets.at(0));
			return;
		}

		if (params_.use_barenco) {
			barenco_create_op(g, controls, targets.at(0));
			return;
		}
		diagonal_create_op(g, controls, targets.at(0));
	}
#pragma endregion

private:
	void barenco_create_op(gate const& g, std::vector<wire_id> const& controls, wire_id target)
	{
		if (controls.size() + 1u == this->num_qubits()) {
			this->create_qubit(wire_modes::ancilla);
		}
		switch (g.id()) {
		// Non-parameterisable gates
		case gate_ids::ncx:
			barenco_decomp(*this, controls, target, params_.controls_threshold);
			return;

		case gate_ids::ncy:
			create_op(gate_lib::sdg, target);
			barenco_decomp(*this, controls, target, params_.controls_threshold);
			create_op(gate_lib::s, target);
			break;

		case gate_ids::ncz: 
			create_op(gate_lib::h, target);
			barenco_decomp(*this, controls, target, params_.controls_threshold);
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

	void diagonal_create_op(gate const& g, std::vector<wire_id> controls, wire_id target)
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
			break;

		case gate_ids::ncry:
			break;

		case gate_ids::ncrz:
			angles.at(angles.size() - 2u) = -g.rotation_angle();
			angles.back() = g.rotation_angle();
			diagonal_synth(*this, controls, angles);
			break;

		default:
			break;
		}
	}

private:
	decomp_params params_;
};

} // namespace detail
#pragma endregion

/*! \brief 
 * 
 * \algtype decomposition
 * \algexpects A network
 * \algreturns A network
 */
template<typename Network>
Network decompose(Network const& network, decomp_params params = {})
{
	using op_type = typename Network::op_type;
	Network result = shallow_duplicate(network);
	detail::decomp_builder decomp(result, params);

	network.foreach_op([&](op_type const& op) {
		if (op.is_one_qubit()) {
			decomp.create_op(op, op.target());
		} else if (op.is_two_qubit()) {
			decomp.create_op(op, op.control(), op.target());
		} else {
			std::vector<wire_id> controls;
			std::vector<wire_id> targets;
			op.foreach_control([&](wire_id control) {
				controls.push_back(control);
			});
			op.foreach_target([&](wire_id target) {
				targets.push_back(target);
			});
			decomp.create_op(op, controls, targets);
		}
	});
	return result;
}

} // namespace tweedledum
