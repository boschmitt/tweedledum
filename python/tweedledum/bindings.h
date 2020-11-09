/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <pybind11/pybind11.h>

// Classical
void init_kitty(pybind11::module& module);

// IR
void init_Circuit(pybind11::module& module);
void init_Instruction(pybind11::module& module);

// Operators
void init_ext_operators(pybind11::module& module);
void init_ising_operators(pybind11::module& module);
void init_meta_operators(pybind11::module& module);
void init_std_operators(pybind11::module& module);

// Passes
void init_Passes(pybind11::module& module);
