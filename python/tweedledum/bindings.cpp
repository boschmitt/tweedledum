/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "bindings.h"

#include "ir/circuit.h"
#include "operators/operators.h"

#include <nanobind/nanobind.h>

NB_MODULE(_tweedledum, module)
{
    namespace nb = nanobind;

    module.doc() = "Binding for the Tweedledum quantum compilation library";

    // Classical
    nb::module_ classical = module.def_submodule("classical", "Tweedledum classical");
    init_kitty(classical);
    init_mockturtle(classical);
    init_classical_utils(classical);

    // IR
    nb::module_ ir = module.def_submodule("ir", "Tweedledum intermediate representation");
    init_Instruction(ir);
    init_Circuit(ir);
    init_Wires(ir);

    // Operators
    nb::module_ ops = module.def_submodule("operators", "Tweedledum operators");
    init_ext_operators(ops);
    init_ising_operators(ops);
    init_meta_operators(ops);
    init_std_operators(ops);

    // Passes 
    nb::module_ passes = module.def_submodule("passes", "Tweedledum passes");
    init_Passes(passes);

    // Synthesis 
    nb::module_ synth =
        module.def_submodule("synthesis", "Tweedledum synthesis methods");
    init_Synthesis(synth);

    // Target
    nb::module_ target = module.def_submodule("target", "Tweedledum target");
    init_Device(target);
    init_Mapping(target);

    // Utils
    nb::module_ utils = module.def_submodule("utils", "Tweedledum utility data structures");
    init_utils(utils);
}
