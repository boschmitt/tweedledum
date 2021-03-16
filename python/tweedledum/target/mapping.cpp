/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <pybind11/stl.h>
#include <tweedledum/Target/Mapping.h>

void init_Mapping(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    py::class_<Placement>(module, "Placement")
        .def("v_to_phy", py::overload_cast<>(&Placement::v_to_phy, py::const_))
        .def("phy_to_v", py::overload_cast<>(&Placement::phy_to_v, py::const_));

    py::class_<Mapping>(module, "Mapping")
        .def_readwrite("init_placement", &Mapping::init_placement)
        .def_readwrite("placement", &Mapping::placement);
}
