/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "Operator.h"
#include "Wire.h"

#include <nlohmann/json.hpp>
#include <vector>
#include <memory>

namespace tweedledum {
class Circuit;

struct InstRef {
	static InstRef invalid()
	{
		return {std::numeric_limits<uint32_t>::max()};
	}

private:
	InstRef(uint32_t id) : uid(id) {}

	uint32_t uid;

	friend class Circuit;
};

class Instruction : public Operator {
public:
	auto begin() const
	{
		return wires_.begin();
	}

	auto end() const
	{
		return wires_.end();
	}

	auto begin_children() const
	{
		return children_.begin();
	}

	auto end_children() const
	{
		return children_.end();
	}

	// FIXME: For now, I will assume that all gates are single target and
	// the target is the last wire! Clearly this is a temporary hack as a
	// gate such as SWAP (two targets) will break this (:
	WireRef target() const
	{
		return wires_.back();
	}

	friend void to_json(nlohmann::json& j, Instruction const& inst);

private:
	template<typename OptorType>
	Instruction(OptorType const& optor, std::vector<WireRef> const& wires)
	    : Operator(optor), wires_(wires)
	{}

	template<typename OptorType>
	Instruction(OptorType const& optor, std::vector<WireRef> const& wires,
	    WireRef target)
	    : Operator(optor), wires_(wires)
	{
		wires_.push_back(target);
	}

	std::vector<WireRef> wires_;
	std::vector<InstRef> children_;

	friend class Circuit;

	// I not sure about this:
	struct Allocator : std::allocator<Instruction> {
		template<class U, class... Args>
		void construct(U* p, Args&&... args)
		{
			::new ((void*) p) U(std::forward<Args>(args)...);
		}

		template<class U>
		struct rebind {
			typedef Allocator other;
		};
	};
	friend struct Allocator;
};

inline void print(Instruction const& inst, std::ostream& os, uint32_t indent)
{
	print(static_cast<Operator const&>(inst), os, indent);
	os << fmt::format("{:>{}}controls:", "", indent + 4);
	std::for_each(inst.begin(), inst.end() - 1, [&os](WireRef const& wire) {
		os << fmt::format(
		    " {}{}", wire.polarity() ? "" : "!", wire.uid());
	});
	os << fmt::format(
	    "\n{:>{}}target: {}\n", "", indent + 4, inst.target().uid());
}

} // namespace tweedledum
