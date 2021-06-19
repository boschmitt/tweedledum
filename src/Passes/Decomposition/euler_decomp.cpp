/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
// At this point this is basically a C++ implementaiton of what you see in:
// https://github.com/Qiskit/qiskit-terra/blob/main/qiskit/quantum_info/synthesis/one_qubit_decompose.py

#include "tweedledum/Passes/Decomposition/euler_decomp.h"

#include "tweedledum/Operators/Extension.h"
#include "tweedledum/Operators/Standard.h"
#include "tweedledum/Passes/Utility/shallow_duplicate.h"
#include "tweedledum/Utils/Matrix.h"
#include "tweedledum/Utils/Numbers.h"

#include <cmath>

namespace tweedledum {

namespace {

struct Config {
    enum class Basis
    {
        px,  // Op::P, Op::Rx
        psx, // Op::P, Op::SX
        xyx, // Op::Rx, Op::Ry
        zsx, // Op::Rz, Op::SX
        zsxx, // Op::Rz, Op::X, Op::SX
        zxz, // Op:Rz, Op::Rx
        zyz  // Op::Rz, Op::Ry
    };

    Basis basis;
    bool simplify;
    double atol;

    Config(nlohmann::json const& config)
        : basis(Basis::zyz)
        , atol(1e-12)
    {
        auto euler_cfg = config.find("euler_decomp");
        if (euler_cfg != config.end()) {
            if (euler_cfg->contains("basis")) {
                if (euler_cfg->at("basis") == "px") {
                    basis = Basis::px;
                } else if (euler_cfg->at("basis") == "psx") {
                    basis = Basis::psx;
                } else if (euler_cfg->at("basis") == "xyx") {
                    basis = Basis::xyx;
                } else if (euler_cfg->at("basis") == "zsx") {
                    basis = Basis::zsx;
                } else if (euler_cfg->at("basis") == "zsxx") {
                    basis = Basis::zsxx;
                } else if (euler_cfg->at("basis") == "zxz") {
                    basis = Basis::zxz;
                } else if (euler_cfg->at("basis") == "zyz") {
                    basis = Basis::zyz;
                } else {
                    assert(0);
                }
            }
        }
    }
};

struct Params {
    double theta;
    double lambda;
    double phi;
    double phase;
};

// Bring the 'difference' between two angles into [-pi; pi].
inline double normalize_npi_pi(double angle, double atol = 0.0)
{
    double const signed_pi = std::copysign(numbers::pi, angle);
    double norm = std::fmod(angle + signed_pi, (2 * numbers::pi)) - signed_pi;
    if (std::abs(norm - numbers::pi) < atol) {
        norm = -numbers::pi;
    }
    return norm;
}

inline Params zyz_params(UMatrix const& matrix)
{
    Params params = {0, 0, 0, 0};

    Complex const phase = 1. / std::sqrt(matrix.determinant());
    params.phase = -std::arg(phase);
    UMatrix2 su_mat = phase * matrix;

    params.theta =
      2 * std::atan2(std::abs(su_mat(1, 0)), std::abs(su_mat(0, 0)));
    double const arg0 = std::arg(su_mat(1, 1));
    double const arg1 = std::arg(su_mat(1, 0));
    params.lambda = (arg0 - arg1);
    params.phi = (arg0 + arg1);
    return params;
}

inline Params zxz_params(UMatrix const& matrix)
{
    Params params = zyz_params(matrix);
    params.lambda -= numbers::pi_div_2;
    params.phi += numbers::pi_div_2;
    return params;
}

inline Params px_params(UMatrix const& matrix)
{
    Params params = zyz_params(matrix);
    params.phase -= 0.5 * (params.theta + params.lambda + params.phi);
    return params;
}

inline Params xyx_params(UMatrix const& matrix)
{
    UMatrix2 zyz_matrix;
    zyz_matrix <<
      0.5 * (matrix(0, 0) + matrix(0, 1) + matrix(1, 0) + matrix(1, 1)),
      0.5 * (matrix(0, 0) - matrix(0, 1) + matrix(1, 0) - matrix(1, 1)),
      0.5 * (matrix(0, 0) + matrix(0, 1) - matrix(1, 0) - matrix(1, 1)),
      0.5 * (matrix(0, 0) - matrix(0, 1) - matrix(1, 0) + matrix(1, 1));
    Params params = zyz_params(zyz_matrix);
    double const phi = normalize_npi_pi(params.phi + numbers::pi);
    double const lambda = normalize_npi_pi(params.lambda + numbers::pi);
    params.phase += (phi + lambda - params.phi  - params.lambda) / 2.;
    params.phi = phi;
    params.lambda = lambda;
    return params;
}

inline double add_rx(Circuit& circuit, Instruction const& inst, double angle, double atol)
{
    double norm = normalize_npi_pi(angle, atol);
    if (std::abs(norm) > atol) {
        circuit.apply_operator(Op::Rx(norm), inst.qubits(), inst.cbits());
        return norm / 2; 
    }
    return 0.0;
}

inline void add_ry(Circuit& circuit, Instruction const& inst, double angle, double atol)
{
    circuit.apply_operator(Op::Ry(angle), inst.qubits(), inst.cbits());
}

inline double add_rz(Circuit& circuit, Instruction const& inst, double angle, double atol)
{
    double norm = normalize_npi_pi(angle, atol);
    if (std::abs(norm) > atol) {
        circuit.apply_operator(Op::Rz(norm), inst.qubits(), inst.cbits());
        return norm / 2; 
    }
    return 0.0;
}

inline double add_p(Circuit& circuit, Instruction const& inst, double angle)
{
    circuit.apply_operator(Op::P(angle), inst.qubits(), inst.cbits());
    return 0.0;
}

inline double add_sx(Circuit& circuit, Instruction const& inst)
{
    circuit.apply_operator(Op::Sx(), inst.qubits(), inst.cbits());
    return 0.0;
}

inline double add_rx_pi_2(Circuit& circuit, Instruction const& inst)
{
    circuit.apply_operator(Op::Rx(numbers::pi_div_2), inst.qubits(), inst.cbits());
    return numbers::pi_div_4;
}

template<typename FnParams, typename FnXY, typename FnXZ>
inline bool circuit_xz_xy(Circuit& circuit, Instruction const& inst,
  Config& cfg, FnParams&& compute_params, FnXZ&& add_rx_rz, FnXY&& add_rx_ry)
{
    Params params = compute_params(inst.matrix().value());
    double global_phase = params.phase - ((params.phi + params.lambda) / 2);
    if (std::abs(params.theta) < cfg.atol) {
        double total = params.phi + params.lambda;
        circuit.global_phase() += add_rx_rz(circuit, inst, total, cfg.atol);
        return true;
    }
    if (std::abs(params.theta - numbers::pi) < cfg.atol) {
        global_phase += params.phi;
        params.lambda = params.lambda - params.phi;
        params.phi = 0;
    }
    global_phase += add_rx_rz(circuit, inst, params.lambda, cfg.atol);
    // Do not optimized this one:
    add_rx_ry(circuit, inst, params.theta, std::numeric_limits<double>::min());
    global_phase += add_rx_rz(circuit, inst, params.phi, cfg.atol);
    circuit.global_phase() += global_phase;
    return true;
}

template<typename FnParams, typename FnP, typename FnX>
inline bool circuit_pz_xsx(Circuit& circuit, Instruction const& inst,
  Config& cfg, FnParams&& compute_params, FnP&& add_phase, FnX&& add_x)
{
    Params params = compute_params(inst.matrix().value());
    circuit.global_phase() += params.phase;
    if (std::abs(params.theta) < cfg.atol) {
        circuit.global_phase() += add_phase(circuit, inst, params.lambda + params.phi);
        return true;
    }
    if (std::abs(params.theta - numbers::pi_div_2) < cfg.atol) {
        circuit.global_phase() += add_phase(circuit, inst, params.lambda - numbers::pi_div_2);
        add_x(circuit, inst);
        circuit.global_phase() += add_phase(circuit, inst, params.phi + numbers::pi_div_2);
        return true;
    }
    if (std::abs(params.theta - numbers::pi) < cfg.atol) {
        circuit.global_phase() += params.lambda;
        params.phi -= params.lambda;
        params.lambda = 0;
    }
    circuit.global_phase() -= numbers::pi_div_2;
    circuit.global_phase() += add_phase(circuit, inst, params.lambda);
    circuit.global_phase() += add_x(circuit, inst);
    circuit.global_phase() += add_phase(circuit, inst, params.theta + numbers::pi);
    circuit.global_phase() += add_x(circuit, inst);
    circuit.global_phase() += add_phase(circuit, inst, params.phi + numbers::pi);
    return true;
}

inline bool decompose(Circuit& circuit, Instruction const& inst, Config& cfg)
{
    switch (cfg.basis) {
    case Config::Basis::px:
        circuit_pz_xsx(circuit, inst, cfg, px_params, add_p, add_rx_pi_2);
        break;

    case Config::Basis::psx:
        circuit_pz_xsx(circuit, inst, cfg, px_params, add_p, add_sx);
        break;

    case Config::Basis::xyx:
        circuit_xz_xy(circuit, inst, cfg, xyx_params, add_rx, add_ry);
        break;

    case Config::Basis::zsx:
        circuit_pz_xsx(circuit, inst, cfg, px_params, add_p, add_sx);
        break;

    case Config::Basis::zsxx:
        break;

    case Config::Basis::zxz:
        circuit_xz_xy(circuit, inst, cfg, zxz_params, add_rz, add_rx);
        break;

    case Config::Basis::zyz:
        circuit_xz_xy(circuit, inst, cfg, zyz_params, add_rz, add_ry);
        break;

    default:
        return false;
    }
    return true;
}

} // namespace

void euler_decomp(
  Circuit& circuit, Instruction const& inst, nlohmann::json const& config)
{
    Config cfg(config);
    decompose(circuit, inst, cfg);
}

Circuit euler_decomp(Circuit const& original, nlohmann::json const& config)
{
    Config cfg(config);
    Circuit decomposed = shallow_duplicate(original);
    original.foreach_instruction([&](Instruction const& inst) {
        if (inst.is_a<Op::Unitary>() && inst.num_qubits() == 1u) {
            decompose(decomposed, inst, cfg);
            return;
        }
        decomposed.apply_operator(inst);
    });
    return decomposed;
}

} // namespace tweedledum
