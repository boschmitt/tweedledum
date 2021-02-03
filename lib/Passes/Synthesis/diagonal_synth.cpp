/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Synthesis/diagonal_synth.h"

#include "tweedledum/Passes/Synthesis/all_linear_synth.h"
#include "tweedledum/Passes/Synthesis/gray_synth.h"
#ifdef _MSC_VER
#include "tweedledum/Utils/Intrinsics.h"
#endif

namespace tweedledum {

namespace {

inline void complement_qubit(uint32_t i, std::vector<Angle>& angles)
{
    uint32_t const size_2 = (angles.size() / 2);
    uint32_t const step = (size_2 >> i);
    for (uint32_t j = 0u; j < size_2; j += step) {
        for (uint32_t k = (j << 1); k < (step + (j << 1)); k += 1) {
            std::swap(angles.at(k), angles.at(step + k));
        }
    }
}

inline std::vector<Angle> fix_angles(
    std::vector<WireRef>& qubits, std::vector<Angle> const& angles)
{
    std::vector<Angle> new_angles;
    std::transform(angles.begin(), angles.end(),
        std::back_inserter(new_angles), [](Angle a) {
            return -a;
        });

    // Normalize qubits polarity
    uint32_t index = 0u;
    for (uint32_t i = qubits.size(); i-- > 0;) {
        if (!qubits.at(index).is_complemented()) {
            index += 1;
            continue;
        }
        qubits.at(index).complement();
        complement_qubit(index, new_angles);
        index += 1;
    }
    return new_angles;
}

inline void fast_hadamard_transform(std::vector<Angle>& angles)
{
    uint32_t k = 0u;
    for (uint32_t m = 1u; m < angles.size(); m <<= 1u) {
        for (uint32_t i = 0u; i < angles.size(); i += (m << 1u)) {
            for (uint32_t j = i, p = k = i + m; j < p; ++j, ++k) {
                Angle t = angles.at(j);
                angles.at(j) += angles.at(k);
                angles.at(k) = t - angles.at(k);
            }
        }
    }
}

}

void diagonal_synth(Circuit& circuit, std::vector<WireRef> qubits,
    std::vector<Angle> const& angles, nlohmann::json const& config)
{
    // Number of angles + 1 needs to be a power of two!
    assert(!angles.empty() && !(angles.size() & (angles.size() - 1)));
    assert(!qubits.empty() && qubits.size() <= 32);
    assert((1u << qubits.size()) == angles.size());
    std::sort(qubits.begin(), qubits.end());

    std::vector<Angle> new_angles = fix_angles(qubits, angles);
    fast_hadamard_transform(new_angles);
    LinearPP parities;
    uint32_t factor = (1 << (qubits.size() - 1));
    for (uint32_t i = 1u; i < new_angles.size(); ++i) {
        if (new_angles.at(i) == 0) {
            continue;
        }
        parities.add_term(i, new_angles.at(i) / factor);
    }
    if (parities.size() == new_angles.size() - 1) {
        all_linear_synth(circuit, qubits, parities);
    } else {
        gray_synth(circuit, qubits, BMatrix::Identity(qubits.size(), qubits.size()), parities, config);
    }
}

Circuit diagonal_synth(std::vector<Angle> const& angles, nlohmann::json const& config)
{
    // Number of angles + 1 needs to be a power of two!
    assert(!angles.empty() && !(angles.size() & (angles.size() - 1)));
    uint32_t num_qubits = __builtin_ctz(angles.size());
    assert(num_qubits <= 32u);

    Circuit circuit;
    // Create the necessary qubits
    std::vector<WireRef> wires;
    wires.reserve(num_qubits);
    for (uint32_t i = 0u; i < num_qubits; ++i) {
        wires.emplace_back(circuit.create_qubit());
    }
    diagonal_synth(circuit, wires, angles, config);
    return circuit;
}

} // namespace tweedledum
