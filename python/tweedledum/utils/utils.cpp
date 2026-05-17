/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <tweedledum/Utils/Cut.h>
#include <tweedledum/Utils/LinPhasePoly.h>

void init_utils(nanobind::module_& module)
{
    using namespace tweedledum;
    namespace nb = nanobind;

    nb::class_<LinPhasePoly>(module, "LinPhasePoly")
        .def(nb::init<>())
        .def("add_term", nb::overload_cast<uint32_t, double const>(&LinPhasePoly::add_term));
        // This is too slow and I have no idea why :(
        // .def("add_term", nb::overload_cast<std::vector<uint32_t> const&, double const>(&LinPhasePoly::add_term));

    nb::class_<Cut>(module, "Cut")
        .def("cbits", &Cut::py_cbits)
        .def("qubits", &Cut::py_qubits)
        .def("instructions", &Cut::py_instructions);
}
