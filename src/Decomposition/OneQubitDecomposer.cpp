/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
// At this point this is basically a C++ implementaiton of what you see in:
// https://github.com/Qiskit/qiskit-terra/blob/main/qiskit/quantum_info/synthesis/one_qubit_decompose.py

#include "tweedledum/Decomposition/OneQubitDecomposer.h"

#include "tweedledum/Operators/Standard/P.h"
#include "tweedledum/Operators/Standard/Rx.h"
#include "tweedledum/Operators/Standard/Ry.h"
#include "tweedledum/Operators/Standard/Rz.h"
#include "tweedledum/Operators/Standard/Sx.h"
#include "tweedledum/Passes/Utility/shallow_duplicate.h"
#include "tweedledum/Utils/Matrix.h"
#include "tweedledum/Utils/Numbers.h"

#include <cmath>

namespace tweedledum {

OneQubitDecomposer::Config::Config(nlohmann::json const& config)
    : basis(Basis::zyz)
    , atol(1e-12)
{
    auto euler_config = config.find("one_qubit_decomp");
    if (euler_config != config.end()) {
        if (euler_config->contains("basis")) {
            if (euler_config->at("basis") == "px") {
                basis = Basis::px;
            } else if (euler_config->at("basis") == "psx") {
                basis = Basis::psx;
            } else if (euler_config->at("basis") == "xyx") {
                basis = Basis::xyx;
            } else if (euler_config->at("basis") == "zsx") {
                basis = Basis::zsx;
            } else if (euler_config->at("basis") == "zsxx") {
                basis = Basis::zsxx;
            } else if (euler_config->at("basis") == "zxz") {
                basis = Basis::zxz;
            } else if (euler_config->at("basis") == "zyz") {
                basis = Basis::zyz;
            } else {
                assert(0);
            }
        }
    }
}

// Bring the 'difference' between two angles into [-pi; pi].
double OneQubitDecomposer::normalize_npi_pi(double angle, double atol)
{
    double const signed_pi = std::copysign(numbers::pi, angle);
    double norm = std::fmod(angle + signed_pi, (2 * numbers::pi)) - signed_pi;
    if (std::abs(norm - numbers::pi) < atol) {
        norm = -numbers::pi;
    }
    return norm;
}

OneQubitDecomposer::Params OneQubitDecomposer::zyz_params(UMatrix const& matrix)
{
    OneQubitDecomposer::Params params = {0, 0, 0, 0};

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

OneQubitDecomposer::Params OneQubitDecomposer::zxz_params(UMatrix const& matrix)
{
    OneQubitDecomposer::Params params = zyz_params(matrix);
    params.lambda -= numbers::pi_div_2;
    params.phi += numbers::pi_div_2;
    return params;
}

OneQubitDecomposer::Params OneQubitDecomposer::px_params(UMatrix const& matrix)
{
    OneQubitDecomposer::Params params = zyz_params(matrix);
    params.phase -= 0.5 * (params.theta + params.lambda + params.phi);
    return params;
}

OneQubitDecomposer::Params OneQubitDecomposer::xyx_params(UMatrix const& matrix)
{
    UMatrix2 zyz_matrix;
    // clang-format off
    zyz_matrix << 
      0.5 * (matrix(0, 0) + matrix(0, 1) + matrix(1, 0) + matrix(1, 1)),
      0.5 * (matrix(0, 0) - matrix(0, 1) + matrix(1, 0) - matrix(1, 1)),
      0.5 * (matrix(0, 0) + matrix(0, 1) - matrix(1, 0) - matrix(1, 1)),
      0.5 * (matrix(0, 0) - matrix(0, 1) - matrix(1, 0) + matrix(1, 1));
    // clang format on
    OneQubitDecomposer::Params params = zyz_params(zyz_matrix);
    double const phi = normalize_npi_pi(params.phi + numbers::pi);
    double const lambda = normalize_npi_pi(params.lambda + numbers::pi);
    params.phase += (phi + lambda - params.phi - params.lambda) / 2.;
    params.phi = phi;
    params.lambda = lambda;
    return params;
}

double OneQubitDecomposer::add_rx(
  Circuit& circuit, Instruction const& inst, double angle, double atol)
{
    double norm = normalize_npi_pi(angle, atol);
    if (std::abs(norm) > atol) {
        circuit.apply_operator(Op::Rx(norm), inst.qubits(), inst.cbits());
        return norm / 2;
    }
    return 0.0;
}

void OneQubitDecomposer::add_ry(Circuit& circuit, Instruction const& inst,
  double angle, [[maybe_unused]] double atol)
{
    circuit.apply_operator(Op::Ry(angle), inst.qubits(), inst.cbits());
}

double OneQubitDecomposer::add_rz(
  Circuit& circuit, Instruction const& inst, double angle, double atol)
{
    double norm = normalize_npi_pi(angle, atol);
    if (std::abs(norm) > atol) {
        circuit.apply_operator(Op::Rz(norm), inst.qubits(), inst.cbits());
        return norm / 2;
    }
    return 0.0;
}

double OneQubitDecomposer::add_p(
  Circuit& circuit, Instruction const& inst, double angle)
{
    circuit.apply_operator(Op::P(angle), inst.qubits(), inst.cbits());
    return 0.0;
}

double OneQubitDecomposer::add_sx(Circuit& circuit, Instruction const& inst)
{
    circuit.apply_operator(Op::Sx(), inst.qubits(), inst.cbits());
    return 0.0;
}

double OneQubitDecomposer::add_rx_pi_2(
  Circuit& circuit, Instruction const& inst)
{
    circuit.apply_operator(
      Op::Rx(numbers::pi_div_2), inst.qubits(), inst.cbits());
    return numbers::pi_div_4;
}

template<typename FnParams, typename FnXY, typename FnXZ>
bool OneQubitDecomposer::circuit_xz_xy(Circuit& circuit,
  Instruction const& inst, FnParams&& compute_params, FnXZ&& add_rx_rz,
  FnXY&& add_rx_ry)
{
    Params params = compute_params(inst.matrix().value());
    double global_phase = params.phase - ((params.phi + params.lambda) / 2);
    if (std::abs(params.theta) < config.atol) {
        double total = params.phi + params.lambda;
        circuit.global_phase() += add_rx_rz(circuit, inst, total, config.atol);
        return true;
    }
    if (std::abs(params.theta - numbers::pi) < config.atol) {
        global_phase += params.phi;
        params.lambda = params.lambda - params.phi;
        params.phi = 0;
    }
    global_phase += add_rx_rz(circuit, inst, params.lambda, config.atol);
    // Do not optimized this one:
    add_rx_ry(circuit, inst, params.theta, std::numeric_limits<double>::min());
    global_phase += add_rx_rz(circuit, inst, params.phi, config.atol);
    circuit.global_phase() += global_phase;
    return true;
}

template<typename FnParams, typename FnP, typename FnX>
bool OneQubitDecomposer::circuit_pz_xsx(Circuit& circuit,
  Instruction const& inst, FnParams&& compute_params, FnP&& add_phase,
  FnX&& add_x)
{
    Params params = compute_params(inst.matrix().value());
    circuit.global_phase() += params.phase;
    if (std::abs(params.theta) < config.atol) {
        circuit.global_phase() +=
          add_phase(circuit, inst, params.lambda + params.phi);
        return true;
    }
    if (std::abs(params.theta - numbers::pi_div_2) < config.atol) {
        circuit.global_phase() +=
          add_phase(circuit, inst, params.lambda - numbers::pi_div_2);
        add_x(circuit, inst);
        circuit.global_phase() +=
          add_phase(circuit, inst, params.phi + numbers::pi_div_2);
        return true;
    }
    if (std::abs(params.theta - numbers::pi) < config.atol) {
        circuit.global_phase() += params.lambda;
        params.phi -= params.lambda;
        params.lambda = 0;
    }
    circuit.global_phase() -= numbers::pi_div_2;
    circuit.global_phase() += add_phase(circuit, inst, params.lambda);
    circuit.global_phase() += add_x(circuit, inst);
    circuit.global_phase() +=
      add_phase(circuit, inst, params.theta + numbers::pi);
    circuit.global_phase() += add_x(circuit, inst);
    circuit.global_phase() +=
      add_phase(circuit, inst, params.phi + numbers::pi);
    return true;
}

bool OneQubitDecomposer::decompose(Circuit& circuit, Instruction const& inst)
{
    switch (config.basis) {
    case Basis::px:
        circuit_pz_xsx(circuit, inst, px_params, add_p, add_rx_pi_2);
        break;

    case Basis::psx:
        circuit_pz_xsx(circuit, inst, px_params, add_p, add_sx);
        break;

    case Basis::xyx:
        circuit_xz_xy(circuit, inst, xyx_params, add_rx, add_ry);
        break;

    case Basis::zsx:
        circuit_pz_xsx(circuit, inst, px_params, add_p, add_sx);
        break;

    case Basis::zsxx:
        break;

    case Basis::zxz:
        circuit_xz_xy(circuit, inst, zxz_params, add_rz, add_rx);
        break;

    case Basis::zyz:
        circuit_xz_xy(circuit, inst, zyz_params, add_rz, add_ry);
        break;

    default:
        return false;
    }
    return true;
}

} // namespace tweedledum
