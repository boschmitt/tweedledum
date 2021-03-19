/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "../my_bool.h"
#include "../nlohmann_json.h"

#include <pybind11/eigen.h>
#include <pybind11/stl.h>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/Synthesis/Synthesis.h>

void init_Synthesis(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    module.def("decomp_synth",
        py::overload_cast<std::vector<uint32_t> const&>(&decomp_synth),
        "Reversible synthesis based on functional decomposition.");

    module.def("decomp_synth", 
        py::overload_cast<Circuit&, std::vector<Qubit> const&, std::vector<Cbit> const&, std::vector<uint32_t> const&>(&decomp_synth),
        "Reversible synthesis based on functional decomposition.");

    module.def("gray_synth",
        py::overload_cast<uint32_t, LinPhasePoly const&, nlohmann::json const&>(&gray_synth),
        py::arg("num_qubits"), py::arg("parities"), py::arg("config") = nlohmann::json(),
        "Synthesis of a CNOT-dihedral circuits.");

    module.def("gray_synth",
        py::overload_cast<Circuit&, std::vector<Qubit> const&, std::vector<Cbit> const&, BMatrix, LinPhasePoly, nlohmann::json const&>(&gray_synth),
        py::arg("circuit"), py::arg("qubits"), py::arg("cbits"), py::arg("linear_trans"), py::arg("parities"), py::arg("config") = nlohmann::json(),
        "Synthesis of a CNOT-dihedral circuits.");

    module.def("lhrs_synth",
        py::overload_cast<mockturtle::xag_network const&, nlohmann::json const&>(&lhrs_synth),
        py::arg("xag"), py::arg("config") = nlohmann::json(),
        "LUT-based Hierarchical Synthesis from a XAG representation.");

    module.def("lhrs_synth",
        py::overload_cast<Circuit&, std::vector<Qubit> const&, std::vector<Cbit> const&, mockturtle::xag_network const&, nlohmann::json const&>(&lhrs_synth),
        py::arg("circuit"), py::arg("qubits"), py::arg("cbits"), py::arg("xag"), py::arg("config") = nlohmann::json(),
        "LUT-based Hierarchical Synthesis inplace from a XAG representation.");

    module.def("linear_synth",
        py::overload_cast<BMatrix const&, nlohmann::json const&>(&linear_synth),
        py::arg("matrix"), py::arg("config") = nlohmann::json(),
        "Synthesis of linear reversible circuits (CNOT synthesis).");

    module.def("linear_synth",
        py::overload_cast<Circuit&, std::vector<Qubit> const&, std::vector<Cbit> const&, BMatrix const&, nlohmann::json const&>(&linear_synth),
        py::arg("circuit"), py::arg("qubits"), py::arg("cbits"), py::arg("matrix"), py::arg("config") = nlohmann::json(),
        "Reversible synthesis based on functional decomposition.");

    module.def("pkrm_synth",
        py::overload_cast<kitty::dynamic_truth_table const&, nlohmann::json const&>(&pkrm_synth),
        py::arg("function"), py::arg("config") = nlohmann::json(),
        "Synthesize a quantum circuit from a function by computing PKRM representation.");

    module.def("pkrm_synth",
        py::overload_cast<Circuit&, std::vector<Qubit> const&, std::vector<Cbit> const&, kitty::dynamic_truth_table const&, nlohmann::json const&>(&pkrm_synth),
        py::arg("circuit"), py::arg("qubits"), py::arg("cbits"), py::arg("function"), py::arg("config") = nlohmann::json(),
        "Synthesize a quantum circuit inplace from a function by computing PKRM representation.");

    module.def("pprm_synth", 
        py::overload_cast<kitty::dynamic_truth_table const&, nlohmann::json const&>(&pprm_synth),
        py::arg("function"), py::arg("config") = nlohmann::json(),
        "Synthesize a quantum circuit from a function by computing PPRM representation.");

    module.def("pprm_synth", 
        py::overload_cast<Circuit&, std::vector<Qubit> const&, std::vector<Cbit> const&, kitty::dynamic_truth_table const&, nlohmann::json const&>(&pprm_synth),
        py::arg("circuit"), py::arg("qubits"), py::arg("cbits"), py::arg("function"), py::arg("config") = nlohmann::json(),
        "Synthesize a quantum circuit inplace from a function by computing PPRM representation.");

    module.def("spectrum_synth", 
        py::overload_cast<kitty::dynamic_truth_table const&, nlohmann::json const&>(&spectrum_synth),
        py::arg("function"), py::arg("config") = nlohmann::json(),
        "Synthesize a quantum circuit from a function by computing the Rademacher-Walsh spectrum.");

    module.def("spectrum_synth", 
        py::overload_cast<Circuit&, std::vector<Qubit> const&, std::vector<Cbit> const&, kitty::dynamic_truth_table const&, nlohmann::json const&>(&spectrum_synth),
        py::arg("circuit"), py::arg("qubits"), py::arg("cbits"), py::arg("function"), py::arg("config") = nlohmann::json(),
        "Synthesize a quantum circuit inplace from a function by computing the Rademacher-Walsh spectrum.");

    module.def("transform_synth", 
        py::overload_cast<std::vector<uint32_t> const&>(&transform_synth),
        "Reversible synthesis based on symbolic transformation.");

    module.def("transform_synth", 
        py::overload_cast<Circuit&, std::vector<Qubit> const&, std::vector<Cbit> const&, std::vector<uint32_t> const&>(&transform_synth),
        "Reversible synthesis based on symbolic transformation.");

    module.def("xag_synth", 
        py::overload_cast<mockturtle::xag_network const&, nlohmann::json const&>(&xag_synth),
        py::arg("xag"), py::arg("config") = nlohmann::json(),
        "Synthesize a quantum circuit from a XAG representation.");

    module.def("xag_synth",
        py::overload_cast<Circuit&, std::vector<Qubit> const&, std::vector<Cbit> const&, mockturtle::xag_network const&, nlohmann::json const&>(&xag_synth),
        py::arg("circuit"), py::arg("qubits"), py::arg("cbits"), py::arg("xag"), py::arg("config") = nlohmann::json(),
        "Synthesize a quantum circuit inplace from a XAG representation.");
}
