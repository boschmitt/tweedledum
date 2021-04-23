/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Target/Device.h"

#include <fstream>

namespace tweedledum {

Device Device::from_edge_list(std::vector<Device::Edge> const& edges)
{
    uint32_t num_qubits = 0;
    for (auto const& [v, w] : edges) {
        num_qubits = std::max(num_qubits, v);
        num_qubits = std::max(num_qubits, w);
    }
    num_qubits += 1;
    Device device(num_qubits, "");
    for (auto const& [v, w] : edges) {
        device.add_edge(v, w);
    }
    return device;
}

Device Device::from_json(nlohmann::json const& device_info)
{
    uint32_t const num_qubits = device_info["n_qubits"];
    std::string name = device_info["backend_name"];
    Device device(num_qubits, name);
    for (auto const& value : device_info["coupling_map"]) {
        uint32_t const v = value[0];
        uint32_t const w = value[1];
        device.add_edge(v, w);
    }
    return device;
}

Device Device::from_file(std::string const& filename)
{
    std::ifstream input(filename, std::ios::in);
    nlohmann::json device_info = nlohmann::json::parse(input);
    return from_json(device_info);
}

} // namespace tweedledum
