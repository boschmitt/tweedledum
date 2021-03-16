/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "../nlohmann_json.h"

#include <pybind11/stl.h>
#include <tweedledum/Target/Device.h>

void init_Device(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    py::class_<Device>(module, "Device")
        .def_static("grid", &Device::grid)
        .def_static("path", &Device::path)
        .def_static("ring", &Device::ring)
        .def_static("star", &Device::star)
        .def_static("from_edge_list", &Device::from_edge_list)
        .def_static("from_file", &Device::from_file)
        .def_static("from_json", &Device::from_json);
}
