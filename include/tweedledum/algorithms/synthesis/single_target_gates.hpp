/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#pragma once

#include "gray_synth.hpp"

#include <cstdint>
#include <iostream>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/esop.hpp>
#include <kitty/operations.hpp>
#include <kitty/print.hpp>
#include <kitty/spectral.hpp>
#include <vector>

namespace tweedledum {

struct stg_from_pprm {
	template<class Network>
	void operator()(Network& net, kitty::dynamic_truth_table const& function,
	                std::vector<uint8_t> const& qubit_map)
	{
		const auto num_controls = function.num_vars();
		assert(qubit_map.size()
		       == static_cast<std::size_t>(num_controls) + 1u);

		for (auto const& cube : esop_from_pprm(function)) {
			assert(cube._bits == cube._mask); /* PPRM property */
			std::vector<uint32_t> controls;
			auto bits = cube._bits;
			for (auto v = 0; v < num_controls; ++v) {
				if (bits & 1) {
					controls.push_back(qubit_map[v]);
				}
				bits >>= 1;
			}
			net.add_toffoli(controls, {qubit_map.back()});
		}
	}
};

struct stg_from_spectrum {
	template<class Network>
	void operator()(Network& net, kitty::dynamic_truth_table const& function,
	                std::vector<uint8_t> const& qubit_map)
	{
		const auto num_controls = function.num_vars();
		assert(qubit_map.size() == num_controls + 1);

		auto g = kitty::extend_to(function, num_controls + 1);
		auto xt = g.construct();
		kitty::create_nth_var(xt, num_controls);
		g &= xt;

		std::vector<uint32_t> parities;
		std::vector<float> angles;

		float nom = 3.14; /* TODO: replace by better PI */
		nom *= (1 << g.num_vars());

		const auto spectrum = kitty::rademacher_walsh_spectrum(g);
		for (auto i = 1u; i < spectrum.size(); ++i) {
			if (spectrum[i] == 0)
				continue;
			parities.push_back(i);
			angles.push_back(nom / spectrum[i]);
		}

		net.add_gate(gate_kinds_t::hadamard, qubit_map.back());
		gray_synth(net, parities, angles, qubit_map);
		net.add_gate(gate_kinds_t::hadamard, qubit_map.back());
	}
};

} /* namespace tweedledum */
