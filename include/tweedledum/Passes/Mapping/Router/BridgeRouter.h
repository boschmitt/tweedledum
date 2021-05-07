/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../../IR/Circuit.h"
#include "../../../IR/Qubit.h"
#include "../../../Target/Device.h"
#include "../../../Target/Mapping.h"
#include "../../../Target/Placement.h"

namespace tweedledum {

class BridgeRouter {
public:
    BridgeRouter(
      Device const& device, Circuit const& original, Placement const& placement)
        : device_(device)
        , original_(original)
        , placement_(placement)
        , delayed_(device_.num_qubits())
    {}

    std::pair<Circuit, Mapping> run();

private:
    std::vector<Qubit> find_unmapped(std::vector<Qubit> const& map) const;

    void place_two_v(Qubit const v0, Qubit const v1);

    void place_one_v(Qubit const v0, Qubit const v1);

    void add_instruction(Instruction const& inst);

    void add_delayed(Qubit const v);

    bool try_add_instruction(InstRef ref, Instruction const& inst);

    Device const& device_;
    Circuit const& original_;
    Circuit* mapped_;
    Placement placement_;
    std::vector<std::vector<InstRef>> delayed_;
};

} // namespace tweedledum
