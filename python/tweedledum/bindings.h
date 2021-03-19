/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <pybind11/pybind11.h>

// Classical
void init_kitty(pybind11::module& module);
void init_mockturtle(pybind11::module& module);
void init_classical_utils(pybind11::module& module);

// IR
void init_Cbit(pybind11::module& module);
void init_Circuit(pybind11::module& module);
void init_Instruction(pybind11::module& module);
void init_Qubit(pybind11::module& module);

// Operators
void init_ext_operators(pybind11::module& module);
void init_ising_operators(pybind11::module& module);
void init_meta_operators(pybind11::module& module);
void init_std_operators(pybind11::module& module);

// Passes
void init_Passes(pybind11::module& module);

// Synthesis
void init_Synthesis(pybind11::module& module);

// Target
void init_Device(pybind11::module& module);
void init_Mapping(pybind11::module& module);

// Utils
void init_utils(pybind11::module& module);
