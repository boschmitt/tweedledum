/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Instruction.h"

#include <vector>

namespace tweedledum {

struct Cut {
    std::vector<Qubit> qubits;
    std::vector<Cbit> cbits;
    std::vector<InstRef> instructions;

    Cut(std::vector<Qubit> const& qs, std::vector<Cbit> const& cs, InstRef ref)
        : qubits(qs)
        , cbits(cs)
        , instructions(1, ref)
    {
        std::sort(qubits.begin(), qubits.end());
        std::sort(cbits.begin(), cbits.end());
    }

    Cut(std::vector<Qubit> const& qs, std::vector<Cbit> const& cs,
      std::vector<InstRef> const& is)
        : qubits(qs)
        , cbits(cs)
        , instructions(is)
    {
        std::sort(qubits.begin(), qubits.end());
        std::sort(cbits.begin(), cbits.end());
    }

    void add_intruction(InstRef ref, Instruction const& instruction)
    {
        std::vector<Qubit> qs = instruction.qubits();
        std::sort(qs.begin(), qs.end());

        std::vector<Qubit> qubits_union;
        std::set_union(qubits.begin(), qubits.end(), qs.begin(), qs.end(),
          std::back_inserter(qubits_union));

        qubits = std::move(qubits_union);
        instructions.emplace_back(ref);
    }

    uint32_t num_qubits() const
    {
        return qubits.size();
    }
    
    bool empty() const
    {
        return instructions.empty();
    }

    bool operator==(Cut const& other) const
    {
        return qubits == other.qubits && cbits == other.cbits
            && instructions == other.instructions;
    }
    friend std::ostream& operator<<(std::ostream& os, Cut const& cut);

    // For python
    std::vector<Qubit> const& py_qubits() const
    {
        return qubits;
    }

    std::vector<Cbit> const& py_cbits() const
    {
        return cbits;
    }

    std::vector<InstRef> const& py_instructions() const
    {
        return instructions;
    }
};

inline std::ostream& operator<<(std::ostream& os, Cut const& cut)
{
    os << "{ " << cut.instructions.front();
    std::for_each(cut.instructions.begin() + 1, cut.instructions.end(),
      [&os](auto ref) { os << ' ' << ref; });
    os << " }";
    return os;
}

inline int32_t try_merge_cuts(Cut& cut0, Cut& cut1, uint32_t max_width)
{
    uint32_t const cut0_size = cut0.qubits.size();
    uint32_t const cut1_size = cut1.qubits.size();
    std::vector<Qubit> qubits_union;
    std::set_union(cut0.qubits.begin(), cut0.qubits.end(), cut1.qubits.begin(),
      cut1.qubits.end(), std::back_inserter(qubits_union));
    if (qubits_union.size() > max_width) {
        if (qubits_union.size() == (cut0_size + cut1_size)) {
            return 0;
        }
        return -1;
    }
    if (cut0.cbits != cut1.cbits) {
        return -1;
    }
    std::copy(cut1.instructions.begin(), cut1.instructions.end(),
      std::back_inserter(cut0.instructions));
    cut1.instructions = std::move(cut0.instructions);
    cut1.qubits = std::move(qubits_union);
    return 1;
}

} // namespace tweedledum
