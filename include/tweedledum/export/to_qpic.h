/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../ir/Circuit.h"

#include <algorithm>
#include <fmt/format.h>
#include <ostream>

namespace tweedledum {

namespace detail {
inline void to_qpic_controls(std::ostream& os, Instruction const& inst)
{
	std::for_each(inst.begin(), inst.end() - 1, [&os](WireRef const& wire) {
		os << fmt::format("{}id{} ",
		    wire.polarity() == WireRef::Polarity::positive ? "" : "-",
		    wire.uid());
	});
}
} // namespace detail

inline void to_qpic(std::ostream& os, Instruction const& inst)
{
	if (inst.is<GateLib::TruthTable>()) {
		auto tt = inst.cast<GateLib::TruthTable>();
		detail::to_qpic_controls(os, inst);
		os << fmt::format(
		    "G {{{}}} +id{}\n", tt.name(), inst.target().uid());
		return;
	}
	os << fmt::format("id{} G {{{}}} ", inst.target().uid(), inst.kind());
	detail::to_qpic_controls(os, inst);
	os << '\n';
}

inline void to_qpic(std::ostream& os, Circuit const& circuit)
{
	// TODO: Header
	std::for_each(
	    circuit.begin_wire(), circuit.end_wire(), [&os](Wire const& wire) {
		    os << fmt::format(
		        "id{0} W \\detokenize{{{1}}} \\detokenize{{{1}}}",
		        wire.uid, wire.name);
		    if (wire.kind == Wire::Kind::classical) {
			    os << "cwire";
		    }
		    os << '\n';
	    });
	// Iterate over instructions
	for (auto const& inst : circuit) {
		to_qpic(os, inst);
	}
}

} // namespace tweedledum
