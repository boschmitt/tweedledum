/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/diagonal_synth.h"

#include "tweedledum/Synthesis/all_linear_synth.h"
#include "tweedledum/Synthesis/gray_synth.h"
#ifdef _MSC_VER
    #include "tweedledum/Utils/Intrinsics.h"
#endif

namespace tweedledum {

namespace {

inline std::vector<double> fix_angles(
  std::vector<Qubit>& qubits, std::vector<double> const& angles)
{
    std::vector<double> new_angles;
    std::transform(angles.begin(), angles.end(), std::back_inserter(new_angles),
      [](double a) { return -a; });
    return new_angles;
}

inline void fast_hadamard_transform(std::vector<double>& angles)
{
    uint32_t k = 0u;
    for (uint32_t m = 1u; m < angles.size(); m <<= 1u) {
        for (uint32_t i = 0u; i < angles.size(); i += (m << 1u)) {
            for (uint32_t j = i, p = k = i + m; j < p; ++j, ++k) {
                double t = angles.at(j);
                angles.at(j) += angles.at(k);
                angles.at(k) = t - angles.at(k);
            }
        }
    }
}

} // namespace

void diagonal_synth(Circuit& circuit, std::vector<Qubit> qubits,
  std::vector<Cbit> const& cbits, std::vector<double> const& angles,
  nlohmann::json const& config)
{
    // Number of angles + 1 needs to be a power of two!
    assert(!angles.empty() && !(angles.size() & (angles.size() - 1)));
    assert(!qubits.empty() && qubits.size() <= 32);
    assert((1u << qubits.size()) == angles.size());

    std::reverse(qubits.begin(), qubits.end());
    std::vector<double> new_angles = fix_angles(qubits, angles);
    fast_hadamard_transform(new_angles);
    LinPhasePoly phase_parities;
    uint32_t factor = (1 << (qubits.size() - 1));
    for (uint32_t i = 1u; i < new_angles.size(); ++i) {
        if (new_angles.at(i) == 0) {
            continue;
        }
        phase_parities.add_term(i, new_angles.at(i) / factor);
    }
    if (phase_parities.size() == new_angles.size() - 1) {
        all_linear_synth(circuit, qubits, cbits, phase_parities);
    } else {
        gray_synth(circuit, qubits, cbits,
          BMatrix::Identity(qubits.size(), qubits.size()), phase_parities,
          config);
    }
}

Circuit diagonal_synth(
  std::vector<double> const& angles, nlohmann::json const& config)
{
    // Number of angles + 1 needs to be a power of two!
    assert(!angles.empty() && !(angles.size() & (angles.size() - 1)));
    uint32_t num_qubits = __builtin_ctz(angles.size());
    assert(num_qubits <= 32u);

    Circuit circuit;
    std::vector<Qubit> qubits;
    qubits.reserve(num_qubits);
    for (uint32_t i = 0u; i < num_qubits; ++i) {
        qubits.emplace_back(circuit.create_qubit());
    }
    diagonal_synth(circuit, qubits, {}, angles, config);
    return circuit;
}

} // namespace tweedledum
