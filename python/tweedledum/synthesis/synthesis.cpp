/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "../my_bool.h"
#include "../nlohmann_json.h"

#include <nanobind/eigen/dense.h>
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/vector.h>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/Synthesis/Synthesis.h>

void init_Synthesis(nanobind::module_& module)
{
    using namespace tweedledum;
    namespace nb = nanobind;

    using BoolArray2D = nb::ndarray<bool, nb::ndim<2>, nb::device::cpu>;

    module.def("a_star_swap_synth", 
        nb::overload_cast<Device const&, std::vector<uint32_t> const&, std::vector<uint32_t> const&, nlohmann::json const&>(&a_star_swap_synth),
        nb::arg("device"), nb::arg("init_cfg"), nb::arg("final_cfg"), nb::arg("config") = nlohmann::json(),
        "Synthesize a quantum swap circuit.");

    module.def("cx_dihedral_synth",
        [](BoolArray2D linear_trans, LinPhasePoly const& parities, nlohmann::json const& config) {
            return cx_dihedral_synth(python::to_bmatrix(linear_trans), parities, config);
        },
        nb::arg("linear_trans"), nb::arg("parities"), nb::arg("config") = nlohmann::json(),
        "Optimal synthesis of a CNOT-dihedral circuits.");

    module.def("cx_dihedral_synth",
        [](Circuit& circuit, std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits, BoolArray2D linear_trans, LinPhasePoly parities, nlohmann::json const& config) {
            cx_dihedral_synth(circuit, qubits, cbits, python::to_bmatrix(linear_trans), std::move(parities), config);
        },
        nb::arg("circuit"), nb::arg("qubits"), nb::arg("cbits"), nb::arg("linear_trans"), nb::arg("parities"), nb::arg("config") = nlohmann::json(),
        "Optimal synthesis of a CNOT-dihedral circuits.");

    module.def("decomp_synth",
        nb::overload_cast<std::vector<uint32_t> const&>(&decomp_synth),
        "Reversible synthesis based on functional decomposition.");

    module.def("decomp_synth", 
        nb::overload_cast<Circuit&, std::vector<Qubit> const&, std::vector<Cbit> const&, std::vector<uint32_t> const&>(&decomp_synth),
        "Reversible synthesis based on functional decomposition.");

    module.def("diagonal_synth",
        nb::overload_cast<std::vector<double> const&, nlohmann::json const&>(&diagonal_synth),
        nb::arg("angles"), nb::arg("config") = nlohmann::json(),
        "Synthesis of diagonal unitary matrices.");

    module.def("diagonal_synth", 
        nb::overload_cast<Circuit&, std::vector<Qubit>, std::vector<Cbit> const&, std::vector<double> const&, nlohmann::json const&>(&diagonal_synth),
        nb::arg("circuit"), nb::arg("qubits"), nb::arg("cbits"), nb::arg("angles"), nb::arg("config") = nlohmann::json(),
        "Synthesis of diagonal unitary matrices.");

    module.def("gray_synth",
        nb::overload_cast<uint32_t, LinPhasePoly const&, nlohmann::json const&>(&gray_synth),
        nb::arg("num_qubits"), nb::arg("parities"), nb::arg("config") = nlohmann::json(),
        "Synthesis of a CNOT-dihedral circuits.");

    module.def("gray_synth",
        [](Circuit& circuit, std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits, BoolArray2D linear_trans, LinPhasePoly parities, nlohmann::json const& config) {
            gray_synth(circuit, qubits, cbits, python::to_bmatrix(linear_trans), std::move(parities), config);
        },
        nb::arg("circuit"), nb::arg("qubits"), nb::arg("cbits"), nb::arg("linear_trans"), nb::arg("parities"), nb::arg("config") = nlohmann::json(),
        "Synthesis of a CNOT-dihedral circuits.");

    module.def("lhrs_synth",
        nb::overload_cast<mockturtle::xag_network const&, nlohmann::json const&>(&lhrs_synth),
        nb::arg("xag"), nb::arg("config") = nlohmann::json(),
        "LUT-based Hierarchical Synthesis from a XAG representation.");

    module.def("lhrs_synth",
        nb::overload_cast<Circuit&, std::vector<Qubit> const&, std::vector<Cbit> const&, mockturtle::xag_network const&, nlohmann::json const&>(&lhrs_synth),
        nb::arg("circuit"), nb::arg("qubits"), nb::arg("cbits"), nb::arg("xag"), nb::arg("config") = nlohmann::json(),
        "LUT-based Hierarchical Synthesis inplace from a XAG representation.");

    module.def("linear_synth",
        [](BoolArray2D matrix, nlohmann::json const& config) {
            return linear_synth(python::to_bmatrix(matrix), config);
        },
        nb::arg("matrix"), nb::arg("config") = nlohmann::json(),
        "Synthesis of linear reversible circuits (CNOT synthesis).");

    module.def("linear_synth",
        [](Circuit& circuit, std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits, BoolArray2D matrix, nlohmann::json const& config) {
            linear_synth(circuit, qubits, cbits, python::to_bmatrix(matrix), config);
        },
        nb::arg("circuit"), nb::arg("qubits"), nb::arg("cbits"), nb::arg("matrix"), nb::arg("config") = nlohmann::json(),
        "Reversible synthesis based on functional decomposition.");

    module.def("pkrm_synth",
        nb::overload_cast<kitty::dynamic_truth_table const&, nlohmann::json const&>(&pkrm_synth),
        nb::arg("function"), nb::arg("config") = nlohmann::json(),
        "Synthesize a quantum circuit from a function by computing PKRM representation.");

    module.def("pkrm_synth",
        nb::overload_cast<Circuit&, std::vector<Qubit> const&, std::vector<Cbit> const&, kitty::dynamic_truth_table const&, nlohmann::json const&>(&pkrm_synth),
        nb::arg("circuit"), nb::arg("qubits"), nb::arg("cbits"), nb::arg("function"), nb::arg("config") = nlohmann::json(),
        "Synthesize a quantum circuit inplace from a function by computing PKRM representation.");

    module.def("pprm_synth", 
        nb::overload_cast<kitty::dynamic_truth_table const&, nlohmann::json const&>(&pprm_synth),
        nb::arg("function"), nb::arg("config") = nlohmann::json(),
        "Synthesize a quantum circuit from a function by computing PPRM representation.");

    module.def("pprm_synth", 
        nb::overload_cast<Circuit&, std::vector<Qubit> const&, std::vector<Cbit> const&, kitty::dynamic_truth_table const&, nlohmann::json const&>(&pprm_synth),
        nb::arg("circuit"), nb::arg("qubits"), nb::arg("cbits"), nb::arg("function"), nb::arg("config") = nlohmann::json(),
        "Synthesize a quantum circuit inplace from a function by computing PPRM representation.");

    module.def("sat_linear_synth",
        [](BoolArray2D matrix, nlohmann::json const& config) {
            return sat_linear_synth(python::to_bmatrix(matrix), config);
        },
        nb::arg("matrix"), nb::arg("config") = nlohmann::json(),
        "SAT-based synthesis of linear reversible circuits (CNOT synthesis).");

    module.def("sat_linear_synth",
        [](Circuit& circuit, std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits, BoolArray2D matrix, nlohmann::json const& config) {
            sat_linear_synth(circuit, qubits, cbits, python::to_bmatrix(matrix), config);
        },
        nb::arg("circuit"), nb::arg("qubits"), nb::arg("cbits"), nb::arg("matrix"), nb::arg("config") = nlohmann::json(),
        "SAT-based synthesis of linear reversible circuits (CNOT synthesis).");

    module.def("sat_linear_synth",
        [](Device const& device, BoolArray2D matrix, nlohmann::json const& config) {
            return sat_linear_synth(device, python::to_bmatrix(matrix), config);
        },
        nb::arg("device"), nb::arg("matrix"), nb::arg("config") = nlohmann::json(),
        "SAT-based synthesis of linear reversible circuits (CNOT synthesis).");

    module.def("sat_linear_synth",
        [](Device const& device, Circuit& circuit, BoolArray2D matrix, nlohmann::json const& config) {
            sat_linear_synth(device, circuit, python::to_bmatrix(matrix), config);
        },
        nb::arg("device"), nb::arg("circuit"), nb::arg("matrix"), nb::arg("config") = nlohmann::json(),
        "SAT-based synthesis of linear reversible circuits (CNOT synthesis).");

    module.def("sat_swap_synth", 
        nb::overload_cast<Device const&, std::vector<uint32_t> const&, std::vector<uint32_t> const&, nlohmann::json const&>(&sat_swap_synth),
        nb::arg("device"), nb::arg("init_cfg"), nb::arg("final_cfg"), nb::arg("config") = nlohmann::json(),
        "Synthesize a quantum swap circuit.");

    module.def("spectrum_synth", 
        nb::overload_cast<kitty::dynamic_truth_table const&, nlohmann::json const&>(&spectrum_synth),
        nb::arg("function"), nb::arg("config") = nlohmann::json(),
        "Synthesize a quantum circuit from a function by computing the Rademacher-Walsh spectrum.");

    module.def("spectrum_synth", 
        nb::overload_cast<Circuit&, std::vector<Qubit> const&, std::vector<Cbit> const&, kitty::dynamic_truth_table const&, nlohmann::json const&>(&spectrum_synth),
        nb::arg("circuit"), nb::arg("qubits"), nb::arg("cbits"), nb::arg("function"), nb::arg("config") = nlohmann::json(),
        "Synthesize a quantum circuit inplace from a function by computing the Rademacher-Walsh spectrum.");

    module.def("steiner_gauss_synth",
        [](Device const& device, BoolArray2D matrix, nlohmann::json const& config) {
            return steiner_gauss_synth(device, python::to_bmatrix(matrix), config);
        },
        nb::arg("device"), nb::arg("matrix"), nb::arg("config") = nlohmann::json(),
        "Synthesis of coupled constrained linear reversible circuits (CNOT synthesis).");

    module.def("steiner_gauss_synth",
        [](Circuit& circuit, Device const& device, BoolArray2D matrix, nlohmann::json const& config) {
            steiner_gauss_synth(circuit, device, python::to_bmatrix(matrix), config);
        },
        nb::arg("circuit"), nb::arg("device"), nb::arg("matrix"), nb::arg("config") = nlohmann::json(),
        "Synthesis of coupled constrained linear reversible circuits (CNOT synthesis).");

    module.def("transform_synth", 
        nb::overload_cast<std::vector<uint32_t> const&>(&transform_synth),
        "Reversible synthesis based on symbolic transformation.");

    module.def("transform_synth", 
        nb::overload_cast<Circuit&, std::vector<Qubit> const&, std::vector<Cbit> const&, std::vector<uint32_t> const&>(&transform_synth),
        "Reversible synthesis based on symbolic transformation.");

    module.def("xag_synth", 
        nb::overload_cast<mockturtle::xag_network const&, nlohmann::json const&>(&xag_synth),
        nb::arg("xag"), nb::arg("config") = nlohmann::json(),
        "Synthesize a quantum circuit from a XAG representation.");

    module.def("xag_synth",
        nb::overload_cast<Circuit&, std::vector<Qubit> const&, std::vector<Cbit> const&, mockturtle::xag_network const&, nlohmann::json const&>(&xag_synth),
        nb::arg("circuit"), nb::arg("qubits"), nb::arg("cbits"), nb::arg("xag"), nb::arg("config") = nlohmann::json(),
        "Synthesize a quantum circuit inplace from a XAG representation.");
}
