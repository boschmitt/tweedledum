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
		return do_create_qubit(name);
	}

	WireRef create_qubit()
	{
		std::string const name = fmt::format("__dum_q{}", num_qubits());
		return do_create_qubit(name);
	}

	WireRef request_ancilla()
	{
		if (free_ancillae_.empty()) {
			return do_create_qubit(
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

	auto begin() const
	{
		return instruction_.begin();
	}

	auto end() const
	{
		return instruction_.end();
	}

	template<typename OptorType>
	void create_instruction(OptorType const& optor,
	    std::vector<WireRef> const& controls, WireRef target)
	{
		instruction_.push_back({optor, controls, target});
	}

	template<typename OptorType>
	void create_instruction(
	    OptorType const& optor, std::vector<WireRef> const& wires)
	{
		instruction_.push_back({optor, wires});
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
	std::string const name_;
	std::vector<Instruction> instruction_;
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
