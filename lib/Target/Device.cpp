/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Target/Device.h"

#include <fstream>

namespace tweedledum {

Device read_device_from_json(std::string const& filename)
{
    std::ifstream input(filename, std::ios::in);
    nlohmann::json device_info = nlohmann::json::parse(input);
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

} // namespace tweedledum
