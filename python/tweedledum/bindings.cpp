/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "bindings.h"

#include "IR/Circuit.h"
#include "Operators/Operators.h"
#include "Passes/Passes.h"

#include <pybind11/pybind11.h>

PYBIND11_MODULE(libPyTweedledum, module)
{
    namespace py = pybind11;

    module.doc() = "Binding for the Tweedledum quantum compilation library";

    // Classical
    py::module classical = module.def_submodule("Classical", "Tweedledum classical");
    init_kitty(classical);
    init_mockturtle(classical);

    // IR
    py::module IR = module.def_submodule("IR", "Tweedledum intermediate representation");
    init_Instruction(IR);
    init_Circuit(IR);

    // Operators
    py::module Ops = module.def_submodule("Operators", "Tweedledum operators");
    init_ext_operators(Ops);
    init_ising_operators(Ops);
    init_meta_operators(Ops);
    init_std_operators(Ops);

    // Passes 
    py::module Passes = module.def_submodule("Passes", "Tweedledum passes");
    init_Passes(Passes);
}
