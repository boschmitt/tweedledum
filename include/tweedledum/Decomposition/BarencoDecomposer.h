/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../IR/Instruction.h"

#include <nlohmann/json.hpp>

namespace tweedledum {

class BarencoDecomposer {
public:
    struct Config {
        uint32_t controls_threshold;
        uint32_t max_qubits;
        Qubit locked;
        Operator compute_op;
        Operator cleanup_op;

        Config(nlohmann::json const& config);
    };

    Config config;

    BarencoDecomposer(nlohmann::json const& config = {})
        : config(config)
    {}

    bool decompose(Circuit& circuit, Instruction const& inst);

private:
    inline std::vector<Qubit> get_workspace(
      Circuit& circuit, std::vector<Qubit> const& qubits);

    inline void v_dirty(Circuit& circuit, Operator const& op,
      std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits);

    inline void v_clean(Circuit& circuit, Operator const& op,
      std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits);

    inline void dirty_ancilla(Circuit& circuit, Operator const& op,
      std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits);

    inline void clean_ancilla(Circuit& circuit, Operator const& op,
      std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits);
};

} // namespace tweedledum
