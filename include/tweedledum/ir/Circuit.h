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

	std::string_view name() const
	{
		return name_;
	}

	friend void to_json(nlohmann::json& j, Circuit const& circuit);

private:
	std::string const name_;
	std::vector<Instruction> instruction_;
};

inline void print(Circuit const& circuit, std::ostream& os, uint32_t indent)
{
	os << fmt::format("{:>{}}Circuit: {}\n", "", indent, circuit.name());
	for (auto const& inst : circuit) {
		print(inst, os, indent + 4);
	}
}

} // namespace tweedledum
