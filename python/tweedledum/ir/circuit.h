/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../operators/operators.h"

#include <pybind11/pybind11.h>
#include <tweedledum/IR/Cbit.h>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/IR/Qubit.h>
#include <vector>

namespace tweedledum {

template<>
inline InstRef Circuit::apply_operator(pybind11::object const& obj,
    std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits)
{
    return this->apply_operator(python::PyOperator(obj), qubits, cbits);
}

} // namespace tweedledum
