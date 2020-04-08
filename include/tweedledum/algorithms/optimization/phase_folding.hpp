/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate.hpp"
#include "../../networks/wire.hpp"
#include "../../utils/angle.hpp"
#include "../../utils/parity_terms.hpp"

#include <cstdint>
#include <limits>
#include <vector>

namespace tweedledum {

/*! \brief TODO
 */
template<typename Network>
Network phase_folding(Network const& original)
{
	using op_type = typename Network::op_type;
	using sum_type = std::vector<uint32_t>;
	constexpr uint32_t qid_max = std::numeric_limits<uint32_t>::max();

	Network optmized;
	uint32_t num_path_vars = 1u;
	std::vector<uint32_t> wire_to_qid(original.num_wires(), qid_max);
	std::vector<sum_type> qubit_pathsum;

	original.foreach_wire([&](wire::id w_id, std::string_view name) {
		if (!w_id.is_qubit()) {
			optmized.create_cbit(name);
			return;
		};
		optmized.create_qubit(name);
		wire_to_qid.at(w_id) = qubit_pathsum.size();
		qubit_pathsum.emplace_back(1u, (num_path_vars++ << 1));
	});

	parity_terms<sum_type> parities;
	original.foreach_op([&](op_type const& op) {
		uint32_t t_qid = wire_to_qid.at(op.target());
		if (op.axis() == rot_axis::z) {
			parities.add_term(qubit_pathsum.at(t_qid), op.rotation_angle());
		}
		if (op.id() == gate_ids::x) {
			if (qubit_pathsum.at(t_qid).at(0) == 1u) {
				qubit_pathsum.at(t_qid).erase(qubit_pathsum.at(t_qid).begin());
				return;
			}
			qubit_pathsum.at(t_qid).insert(qubit_pathsum.at(t_qid).begin(), 1u);
			return;
		}
		if (op.id() == gate_ids::cx) {
			uint32_t c_qid = wire_to_qid.at(op.control());
			std::vector<uint32_t> optmized;
			std::set_symmetric_difference(qubit_pathsum.at(c_qid).begin(),
			                              qubit_pathsum.at(c_qid).end(),
			                              qubit_pathsum.at(t_qid).begin(),
			                              qubit_pathsum.at(t_qid).end(),
			                              std::back_inserter(optmized));
			qubit_pathsum.at(t_qid) = optmized;
			return;
		}
		if (op.id() == gate_ids::swap) {
			uint32_t t1_qid = wire_to_qid.at(op.target(1u));
			std::swap(qubit_pathsum.at(t_qid), qubit_pathsum.at(t1_qid));
			return;
		}
		qubit_pathsum.at(t_qid).clear();
		qubit_pathsum.at(t_qid).emplace_back((num_path_vars++ << 1));
	});

	original.foreach_op([&](op_type const& op) {
		if (op.axis() == rot_axis::z) {
			return;
		}
		optmized.emplace_op(op);

		// Could do better that recalculate this
		uint32_t t_qid = wire_to_qid.at(op.target());
		if (op.id() == gate_ids::x) {
			if (qubit_pathsum.at(t_qid).at(0) == 1u) {
				qubit_pathsum.at(t_qid).erase(qubit_pathsum.at(t_qid).begin());
			} else {
				qubit_pathsum.at(t_qid).insert(qubit_pathsum.at(t_qid).begin(), 1u);
			}
		} else if (op.id() == gate_ids::cx) {
			uint32_t c_qid = wire_to_qid.at(op.control());
			std::vector<uint32_t> optmized;
			std::set_symmetric_difference(qubit_pathsum.at(c_qid).begin(),
			                              qubit_pathsum.at(c_qid).end(),
			                              qubit_pathsum.at(t_qid).begin(),
			                              qubit_pathsum.at(t_qid).end(),
			                              std::back_inserter(optmized));
			qubit_pathsum.at(t_qid) = optmized;
		} else if (op.id() == gate_ids::swap) {
			uint32_t t1_qid = wire_to_qid.at(op.target(1u));
			std::swap(qubit_pathsum.at(t_qid), qubit_pathsum.at(t1_qid));
			return; // No need to add a angle, would alredy have done it!
		} else {
			qubit_pathsum.at(t_qid).clear();
			qubit_pathsum.at(t_qid).emplace_back((num_path_vars++ << 1));
		}

		// Check if I neeed to add a rotation Z
		angle rot_angle = parities.extract_term(qubit_pathsum.at(t_qid));
		if (rot_angle == sym_angle::zero) {
			return;
		}
		optmized.create_op(gate_lib::rz(rot_angle), op.target());
	});
	return optmized;
}

} // namespace tweedledum
