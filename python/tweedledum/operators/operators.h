/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <nanobind/nanobind.h>
#include <string_view>
#include <tweedledum/IR/Instruction.h>
#include <tweedledum/Operators/All.h>

namespace tweedledum::python {

class PyOperator {
public:
    static constexpr std::string_view kind()
    {
        return "py_op";
    }

    PyOperator(nanobind::object const& handle)
        : handle_(handle)
    {}

    nanobind::object obj() const
    {
        return handle_;
    }

    bool operator==(PyOperator const& other) const
    {
        return handle_.is(other.handle_);
    }

private:
    nanobind::object handle_;
};

} // namespace tweedledum::python
