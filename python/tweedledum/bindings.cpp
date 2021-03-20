/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "bindings.h"

#include "ir/circuit.h"
#include "operators/operators.h"

#include <pybind11/pybind11.h>

PYBIND11_MODULE(_tweedledum, module)
{
    namespace py = pybind11;

    module.doc() = "Binding for the Tweedledum quantum compilation library";

    // Classical
    py::module classical = module.def_submodule("classical", "Tweedledum classical");
    init_kitty(classical);
    init_mockturtle(classical);
    init_classical_utils(classical);

    // IR
    py::module ir = module.def_submodule("ir", "Tweedledum intermediate representation");
    init_Cbit(ir);
    init_Qubit(ir);
    init_Instruction(ir);
    init_Circuit(ir);

    // Operators
    py::module ops = module.def_submodule("operators", "Tweedledum operators");
    init_ext_operators(ops);
    init_ising_operators(ops);
    init_meta_operators(ops);
    init_std_operators(ops);

    // Passes 
    py::module passes = module.def_submodule("passes", "Tweedledum passes");
    init_Passes(passes);

    // Synthesis 
    py::module synth =
        module.def_submodule("synthesis", "Tweedledum synthesis methods");
    init_Synthesis(synth);

    // Target
    py::module target = module.def_submodule("target", "Tweedledum target");
    init_Device(target);
    init_Mapping(target);

    // Utils
    py::module utils = module.def_submodule("utils", "Tweedledum utility data structures");
    init_utils(utils);
}
