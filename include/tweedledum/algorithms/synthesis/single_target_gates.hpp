/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#pragma once

#include "gray_synth.hpp"
#include "lin_comb_synth.hpp"

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

struct stg_from_pkrm {
	template<class Network>
	void operator()(Network& net, kitty::dynamic_truth_table const& function,
	                std::vector<uint8_t> const& qubit_map)
	{
		const auto num_controls = function.num_vars();
		assert(qubit_map.size()
		       == static_cast<std::size_t>(num_controls) + 1u);

		for (auto const& cube : esop_from_optimum_pkrm(function)) {
			std::vector<uint32_t> controls, negations;
			auto bits = cube._bits;
			auto mask = cube._mask;
			for (auto v = 0; v < num_controls; ++v) {
				if (mask & 1) {
					controls.push_back(qubit_map[v]);
					if (!(bits & 1))
						negations.push_back(qubit_map[v]);
				}
				bits >>= 1;
				mask >>= 1;
			}
			for (auto n : negations)
				net.add_gate(gate_kinds_t::cx, n);
			net.add_toffoli(controls, {qubit_map.back()});
			for (auto n : negations)
				net.add_gate(gate_kinds_t::cx, n);
		}
	}
};

struct stg_from_spectrum_params {
	enum lin_comb_synth_behavior_t {
		always = 0,
		never = 1,
		complete_spectra = 2
	} lin_comb_synth_behavior{complete_spectra};
	enum lin_comb_synth_strategy_t {
		gray = 0,
		binary = 1
	} lin_comb_synth_strategy{gray};
	gray_synth_params gray_synth_ps{};
};

struct stg_from_spectrum {
	explicit stg_from_spectrum(stg_from_spectrum_params const& ps = {})
	    : ps_(ps)
	{}

	inline double pi()
	{
		static double _pi = std::atan(1) * 4;
		return _pi;
	}

	template<class Network>
	void operator()(Network& net, kitty::dynamic_truth_table const& function,
	                std::vector<uint8_t> const& qubit_map)
	{
		const auto num_controls = function.num_vars();
		assert(qubit_map.size() == num_controls + 1u);

		auto g = kitty::extend_to(function, num_controls + 1);
		auto xt = g.construct();
		kitty::create_nth_var(xt, num_controls);
		g &= xt;

		std::vector<uint32_t> parities;
		std::vector<float> angles;

		float nom = pi();
		nom /= (1 << g.num_vars());

		const auto spectrum = kitty::rademacher_walsh_spectrum(g);
		for (auto i = 1u; i < spectrum.size(); ++i) {
			if (spectrum[i] == 0)
				continue;
			parities.push_back(i);
			angles.push_back(nom * spectrum[i]);
		}

		net.add_gate(gate_kinds_t::hadamard, qubit_map.back());
		if ((ps_.lin_comb_synth_behavior
		     == stg_from_spectrum_params::always)
		    || ((ps_.lin_comb_synth_behavior
		         == stg_from_spectrum_params::complete_spectra)
		        && (parities.size() == spectrum.size() - 1))) {
			if (ps_.lin_comb_synth_strategy
			    == stg_from_spectrum_params::gray)
				lin_comb_synth_gray(net, parities, angles,
				                    qubit_map);
			else
				lin_comb_synth_binary(net, parities, angles,
				                      qubit_map);
		} else
			gray_synth(net, parities, angles, qubit_map,
			           ps_.gray_synth_ps);
		net.add_gate(gate_kinds_t::hadamard, qubit_map.back());
	}

	stg_from_spectrum_params ps_{};
};

} /* namespace tweedledum */
