/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "Operand.h"
#include "Operator.h"

#include <nlohmann/json.hpp>
#include <vector>

namespace tweedledum {

class Instruction : public Operator {
public:
	template<typename OptorType>
	Instruction(OptorType const& optor, std::vector<Operand> const& opnds)
	    : Operator(optor), operands_(opnds)
	{}

	template<typename OptorType>
	Instruction(OptorType const& optor, std::vector<Operand> const& opnds,
	    Operand target)
	    : Operator(optor), operands_(opnds)
	{
		operands_.push_back(target);
	}

	auto begin() const
	{
		return operands_.begin();
	}

	auto end() const
	{
		return operands_.end();
	}

	friend void to_json(nlohmann::json& j, Instruction const& inst);

private:
	std::vector<Operand> operands_;
};

inline void print(Instruction const& inst, std::ostream& os, uint32_t indent)
{
	print(static_cast<Operator const&>(inst), os, indent);
	os << fmt::format("{:>{}}controls:", "", indent + 4);
	std::for_each(inst.begin(), inst.end() - 1, [&os](Operand const& opnd) {
		os << fmt::format(
		    " {}{}", opnd.polarity() ? "" : "!", opnd.uid());
	});
	// FIXME: this is a hack for now (:
	Operand target = *(inst.end() - 1);
	os << fmt::format("\n{:>{}}target: {}\n", "", indent + 4, target.uid());
}

} // namespace tweedledum
