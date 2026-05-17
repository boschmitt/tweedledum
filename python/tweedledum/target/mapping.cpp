/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <tweedledum/Target/Mapping.h>

void init_Mapping(nanobind::module_& module)
{
    using namespace tweedledum;
    namespace nb = nanobind;

    nb::class_<Placement>(module, "Placement")
        .def("v_to_phy", nb::overload_cast<>(&Placement::v_to_phy, nb::const_))
        .def("phy_to_v", nb::overload_cast<>(&Placement::phy_to_v, nb::const_));

    nb::class_<Mapping>(module, "Mapping")
        .def_rw("init_placement", &Mapping::init_placement)
        .def_rw("placement", &Mapping::placement);
}
