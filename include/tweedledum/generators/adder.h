/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../ir/Circuit.h"
#include "../ir/GateLib.h"
#include "../ir/Wire.h"

namespace tweedledum {

// Based on:
//
// Cuccaro, Steven A., et al. "A new quantum ripple-carry addition circuit." 
// arXiv preprint quant-ph/0410184 (2004).
//

// This is a literal translation of the algorithm given in Figure 5 of the 
// paper
inline void carry_ripple_adder_inplace(Circuit& circuit,
    std::vector<WireRef> const& a, std::vector<WireRef> const& b, WireRef carry)
{
	assert(a.size() == b.size());
	uint32_t const n = a.size();
	for (uint32_t i = 1; i < n; ++i) {
		circuit.create_instruction(GateLib::X(), {a[i]}, b[i]);
	}
	WireRef x = circuit.request_ancilla();
	circuit.create_instruction(GateLib::X(), {a[1]}, x);
	circuit.create_instruction(GateLib::X(), {a[0], b[0]}, x);
	circuit.create_instruction(GateLib::X(), {a[2]}, a[1]);
	circuit.create_instruction(GateLib::X(), {x, b[1]}, a[1]);
	circuit.create_instruction(GateLib::X(), {a[3]}, a[2]);

	for (uint32_t i = 2; i < n - 2; ++i) {
		circuit.create_instruction(GateLib::X(), {a[i - 1], b[i]}, a[i]);
		circuit.create_instruction(GateLib::X(), {a[i + 2]}, a[i + 1]);
	}
	circuit.create_instruction(GateLib::X(), {a[n - 3], b[n - 2]}, a[n - 2]);
	circuit.create_instruction(GateLib::X(), {a[n - 1]}, carry);
	circuit.create_instruction(GateLib::X(), {a[n - 2], b[n - 1]}, carry);
	for (uint32_t i = 1; i < n - 1; ++i) {
		circuit.create_instruction(GateLib::X(), {b[i]});
	}

	circuit.create_instruction(GateLib::X(), {x}, b[1]);
	for (uint32_t i = 2; i < n; ++i) {
		circuit.create_instruction(GateLib::X(), {a[i - 1]}, b[i]);
	}

	circuit.create_instruction(GateLib::X(), {a[n - 3], b[n - 2]}, a[n - 2]);

	for (uint32_t i = n - 3; i --> 2;) {
		circuit.create_instruction(GateLib::X(), {a[i - 1], b[i]}, a[i]);
		circuit.create_instruction(GateLib::X(), {a[i + 2]}, a[i + 1]);
		circuit.create_instruction(GateLib::X(), {b[i + 1]});
	}
	circuit.create_instruction(GateLib::X(), {x, b[1]}, a[1]);
	circuit.create_instruction(GateLib::X(), {a[3]}, a[2]);
	circuit.create_instruction(GateLib::X(), {b[2]});
	circuit.create_instruction(GateLib::X(), {a[0], b[0]}, x);
	circuit.create_instruction(GateLib::X(), {a[2]}, a[1]);
	circuit.create_instruction(GateLib::X(), {b[1]});
	circuit.create_instruction(GateLib::X(), {a[1]}, x);
	for (uint32_t i = 0; i < n; ++i) {
		circuit.create_instruction(GateLib::X(), {a[i]}, b[i]);
	}
}

} // namespace tweedledum