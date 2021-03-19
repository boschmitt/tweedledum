/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <pybind11/pybind11.h>
#include <tweedledum/Utils/LinPhasePoly.h>

void init_utils(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    py::class_<LinPhasePoly>(module, "LinPhasePoly")
        .def(py::init<>())
        .def("add_term", py::overload_cast<uint32_t, double const>(&LinPhasePoly::add_term));
        // This is too slow and I have no idea why :(
        // .def("add_term", py::overload_cast<std::vector<uint32_t> const&, double const>(&LinPhasePoly::add_term));
}
