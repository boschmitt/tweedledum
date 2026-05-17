/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <nanobind/nanobind.h>

// Classical
void init_kitty(nanobind::module_& module);
void init_mockturtle(nanobind::module_& module);
void init_classical_utils(nanobind::module_& module);

// IR
void init_Circuit(nanobind::module_& module);
void init_Instruction(nanobind::module_& module);
void init_Wires(nanobind::module_& module);

// Operators
void init_ext_operators(nanobind::module_& module);
void init_ising_operators(nanobind::module_& module);
void init_meta_operators(nanobind::module_& module);
void init_std_operators(nanobind::module_& module);

// Passes
void init_Passes(nanobind::module_& module);

// Synthesis
void init_Synthesis(nanobind::module_& module);

// Target
void init_Device(nanobind::module_& module);
void init_Mapping(nanobind::module_& module);

// Utils
void init_utils(nanobind::module_& module);
