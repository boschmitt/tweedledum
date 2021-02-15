/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../operators/operators.h"

#include <pybind11/pybind11.h>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/IR/Wire.h>
#include <vector>

namespace tweedledum {

template<>
inline InstRef Circuit::apply_operator(pybind11::object const& obj, std::vector<WireRef> const& wires)
{
    return this->apply_operator(python::PyOperator(obj), wires);
}

} // namespace tweedledum
