/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/export/to_json.h"

#include <kitty/dynamic_truth_table.hpp>
#include <mockturtle/algorithms/simulation.hpp>
#include <mockturtle/networks/xag.hpp>
#include <pybind11/pybind11.h>
#include <sstream>
#include <string>
#include <tweedledum/algorithms/synthesis/pkrm_synth.h>
#include <tweedledum/ir/Circuit.h>

namespace tweedledum {

pybind11::object from_json(nlohmann::json const& j)
{
	if (j.is_null()) {
		return pybind11::none();
	}
	if (j.is_boolean()) {
		return pybind11::bool_(j.get<bool>());
	}
	if (j.is_number()) {
		double number = j.get<double>();
		if (number == std::floor(number)) {
			return pybind11::int_(j.get<long>());
		}
		return pybind11::float_(number);
	}
	if (j.is_string()) {
		return pybind11::str(j.get<std::string>());
	}
	if (j.is_array()) {
		pybind11::list obj;
		for (const auto& el : j) {
			obj.append(from_json(el));
		}
		return std::move(obj);
	}
	pybind11::dict obj;
	for (auto it = j.cbegin(); it != j.cend(); ++it) {
		obj[pybind11::str(it.key())] = from_json(it.value());
	}
	return std::move(obj);
}

pybind11::object synthesize_xag(mockturtle::xag_network const& ntk)
{
	using TruthTable = kitty::dynamic_truth_table;

	auto const functions = mockturtle::simulate<TruthTable>(
	    ntk, mockturtle::default_simulator<TruthTable>(ntk.num_pis()));

	// TODO: Properly handle multiple output functions
	Circuit circuit = pkrm_synth(functions.at(0));
	nlohmann::json j = circuit;
	return from_json(j);
}

} // namespace tweedledum

void init_tweedledum(pybind11::module& m)
{
	using namespace tweedledum;
	namespace pybind11 = pybind11;

	m.def("synthesize_xag", &synthesize_xag, "Synthesize XAG");
}
