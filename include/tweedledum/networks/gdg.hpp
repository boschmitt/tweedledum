/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "detail/foreach.hpp"
#include "detail/storage.hpp"
#include "gates/gate_kinds.hpp"

#include <fmt/format.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace tweedledum {

/*! Gate dependecy graph
 *
 * Represent a quantum circuit as a directed acyclic graph. The nodes in the
 * graph are either input/output nodes or operation nodes. All nodes store
 * a gate object, which is defined as a class template parameter, which allows
 * great flexibility in the types supported as gates.
 *
 */
template<typename GateType>
struct gdg {
public:
#pragma region Types and constructors
	using gate_type = GateType;
	using node_type = regular_node<gate_type, 1, 1>;
	using node_ptr_type = typename node_type::pointer_type;
	using storage_type = storage<node_type>;

	gdg()
	    : storage_(std::make_shared<storage_type>())
	{}

	gdg(std::shared_ptr<storage_type> storage)
	    : storage_(storage)
	{}
#pragma endregion

#pragma region I/O and ancillae qubits
private:
	auto create_qubit()
	{
		// std::cout << "Create qubit\n";
		auto qubit_id = static_cast<uint32_t>(storage_->inputs.size());
		auto index = static_cast<uint32_t>(storage_->nodes.size());

		// Create input node
		auto& input_node = storage_->nodes.emplace_back();
		input_node.gate.kind(gate_kinds_t::input);
		input_node.gate.target_qubit(qubit_id);
		storage_->inputs.emplace_back(index, false);

		// Create ouput node
		auto& output_node = storage_->outputs.emplace_back();
		output_node.gate.kind(gate_kinds_t::output);
		output_node.gate.target_qubit(qubit_id);
		output_node.qubit[0].emplace_back(index, true);

		// std::cout << "[done]\n";
		return qubit_id;
	}

public:
	auto add_qubit()
	{
		auto str = fmt::format("q{}", storage_->inputs.size());
		return add_qubit(str);
	}

	auto add_qubit(std::string const& label)
	{
		auto qubit_id = create_qubit();
		label_to_id_.emplace(label, qubit_id);
		id_to_label_.emplace_back(label);
		return qubit_id;
	}
#pragma endregion

#pragma region Structural properties
	auto size() const
	{
		return static_cast<uint32_t>(storage_->nodes.size()
		                             + storage_->outputs.size());
	}

	auto num_gates() const
	{
		return static_cast<uint32_t>(storage_->nodes.size()
		                             - storage_->inputs.size());
	}

	auto num_qubits() const
	{
		return static_cast<uint32_t>(storage_->inputs.size());
	}
#pragma endregion

#pragma region Add single-qubit gates
#pragma endregion

#pragma region Add multi-qubit gates
#pragma endregion

#pragma region Node iterators
#pragma endregion

#pragma region Visited flags
#pragma endregion

private:
	std::unordered_map<std::string, uint32_t> label_to_id_;
	std::vector<std::string> id_to_label_;
	std::shared_ptr<storage_type> storage_;
};

} // namespace tweedledum
