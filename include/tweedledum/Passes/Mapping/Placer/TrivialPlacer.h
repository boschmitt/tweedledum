/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../../IR/Circuit.h"
#include "../../../IR/Qubit.h"
#include "../../../Target/Device.h"
#include "../../../Target/Placement.h"

#include <vector>

namespace tweedledum {

class TrivialPlacer {
public:
    TrivialPlacer(Device const& device, Circuit const& original)
        : device_(device)
        , original_(original)
    {}

    std::optional<Placement> run()
    {
        Placement placement(device_.num_qubits(), original_.num_qubits());
        for (uint32_t i = 0u; i < device_.num_qubits(); ++i) {
            Qubit const qubit = Qubit(i);
            placement.map_v_phy(qubit, qubit);
        }
        return placement;
    }

private:
    Device const& device_;
    Circuit const& original_;
};

/*! \brief Yet to be written.
 */
std::optional<Placement> trivial_place(Device const& device,
    Circuit const& original);

} // namespace tweedledum
