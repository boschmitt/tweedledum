/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate.hpp"
#include "../../networks/wire_id.hpp"
#include "../../utils/angle.hpp"
#include "../generic/rewrite.hpp"
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
	using storage_type = typename Network::storage_type;

	explicit decomp_builder(Network& network, decomp_params const& params)
	    : Network(network)
	    , params_(params)
	{}

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
			create_op(gate_lib::u3(g.rotation_angle() / 2, -sym_angle::pi_half, sym_angle::zero), w1);
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
			if (params_.use_t_par) {
				ccx_tpar(*this, c0, c1, t);
			} else {
				ccx(*this, c0, c1, t);
			}
			return;

		case gate_ids::ncy:
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

	void create_op(gate const& g, std::vector<wire_id> controls, std::vector<wire_id> targets)
	{
		if (params_.gate_set & (1ull << static_cast<uint32_t>(g.id()))) {
			if (controls.size() <= params_.controls_threshold) {
				this->emplace_op(op_type(g, controls, targets));
				return;
			}
		}
		switch (g.id()) {
		// Non-parameterisable gates
		case gate_ids::ncx:
			if (controls.size() + targets.size() == this->num_qubits()) {
				this->create_qubit();
			}
			barenco_decomp(*this, controls, targets.back(), params_.controls_threshold);
			return;

		case gate_ids::ncy:
			// TODO: H
			break;

		case gate_ids::ncz:
			// TODO: H
			break;

		// Parameterisable gates
		case gate_ids::ncrx:
			// TODO: H
			break;

		case gate_ids::ncry:
			// TODO: HS
			break;

		case gate_ids::ncrz:
			// TODO
			break;

		default:
			break;
		}
		this->emplace_op(op_type(g, controls, targets));
	}
#pragma endregion

private:
	decomp_params params_;
};

} // namespace detail
#pragma endregion

/*! \brief 

   \endverbatim
 * 
 * \algtype decomposition
 * \algexpects A network
 * \algreturns A network
 */
template<typename Network>
Network decompose(Network const& network, decomp_params params = {})
{
	Network result = shallow_duplicate(network);
	detail::decomp_builder decomp(result, params);

	network.foreach_op([&](auto const& node) {
		auto const& op = node.operation;
		if (op.gate.is_one_qubit()) {
			decomp.create_op(op.gate, op.target());
		} else if (node.operation.gate.is_two_qubit()) {
			decomp.create_op(op.gate, op.control(), op.target());
		} else {
			std::vector<wire_id> controls;
			std::vector<wire_id> targets;
			op.foreach_control([&](wire_id control) {
				controls.push_back(control);
			});
			op.foreach_target([&](wire_id target) {
				targets.push_back(target);
			});
			decomp.create_op(op.gate, controls, targets);
		}
	});
	return result;
}

} // namespace tweedledum
