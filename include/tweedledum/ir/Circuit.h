/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "Instruction.h"
#include "WireStorage.h"

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string_view>
#include <vector>

namespace tweedledum {

class Circuit : public WireStorage {
public:
	Circuit(std::string_view name) : name_(name) {}

	WireRef create_qubit(std::string_view name)
	{
		last_instruction_.emplace_back(InstRef::invalid());
		return do_create_qubit(name);
	}

	WireRef create_qubit()
	{
		std::string const name = fmt::format("__dum_q{}", num_qubits());
		return create_qubit(name);
	}

	WireRef request_ancilla()
	{
		if (free_ancillae_.empty()) {
			return create_qubit(
			    fmt::format("__dum_a{}", num_qubits()));
		} else {
			WireRef qubit = free_ancillae_.back();
			free_ancillae_.pop_back();
			return qubit;
		}
	}

	void release_ancilla(WireRef qubit)
	{
		free_ancillae_.push_back(qubit);
	}

	uint32_t size() const
	{
		return instruction_.size();
	}
	
	auto begin() const
	{
		return instruction_.cbegin();
	}

	auto end() const
	{
		return instruction_.cend();
	}

	template<typename OptorType>
	InstRef create_instruction(OptorType const& optor,
	    std::vector<WireRef> const& controls, WireRef target)
	{
		Instruction& inst
		    = instruction_.emplace_back(optor, controls, target);
		connect_instruction(inst);
		return InstRef(instruction_.size() - 1);
	}

	template<typename OptorType>
	InstRef create_instruction(
	    OptorType const& optor, std::vector<WireRef> const& wires)
	{
		Instruction& inst = instruction_.emplace_back(optor, wires);
		connect_instruction(inst);
		return InstRef(instruction_.size() - 1);
	}

	static std::string_view kind()
	{
		return "circuit";
	}

	std::string_view name() const
	{
		return name_;
	}

	friend void to_json(nlohmann::json& j, Circuit const& circuit);

private:
	void connect_instruction(Instruction& inst)
	{
		for (WireRef wire : inst) {
			uint32_t w = wire.uid();
			inst.children_.push_back(last_instruction_.at(w));
			last_instruction_.at(w).uid = instruction_.size() - 1;
		}
	}

	std::string const name_;
	std::vector<Instruction, Instruction::Allocator> instruction_;
	std::vector<InstRef> last_instruction_; // last instruction on a wire
	std::vector<WireRef> free_ancillae_; // Should this be here?!
};

inline void print(Circuit const& circuit, std::ostream& os, uint32_t indent)
{
	os << fmt::format("{:>{}}Circuit: {}\n", "", indent, circuit.name());
	for (auto const& inst : circuit) {
		print(inst, os, indent + 4);
	}
}

} // namespace tweedledum
