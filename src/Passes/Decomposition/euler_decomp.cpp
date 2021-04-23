/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
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
    enum class Basis {
        zxz,
        zyz
    };

    Basis basis;

    Config(nlohmann::json const& config)
        : basis(Basis::zyz)
    {
        auto euler_cfg = config.find("euler_decomp");
        if (euler_cfg != config.end()) {
            if (euler_cfg->contains("basis")) {
                if (euler_cfg->at("basis") == "zyz") {
                    basis = Basis::zyz;
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

inline Params zyz_params(UMatrix const& matrix) 
{
    Params params = {0, 0, 0, 0};

    Complex const phase = 1. / std::sqrt(matrix.determinant());
    params.phase = -std::arg(phase);
    UMatrix2 su_mat = phase * matrix;

    params.theta = 2 * std::atan2(std::abs(su_mat(1, 0)), std::abs(su_mat(0, 0)));
    double const arg0 = 2 * std::arg(su_mat(1, 1));
    double const arg1 = 2 * std::arg(su_mat(1, 0));
    params.lambda = (arg0 - arg1) / 2.0;
    params.phi = (arg0 + arg1) / 2.0;
    return params;
}

inline bool decompose(Circuit& circuit, Instruction const& inst, Config& cfg)
{
    Params params = zyz_params(inst.matrix().value());
    if (cfg.basis == Config::Basis::zxz) {
        params.lambda += -numbers::pi_div_2;
        params.phi += numbers::pi_div_2;
    }
    circuit.apply_operator(Op::Rz(params.lambda), inst.qubits(), inst.cbits());
    switch (cfg.basis) {
        case Config::Basis::zxz:
            circuit.apply_operator(Op::Rx(params.theta), inst.qubits(), inst.cbits());
            break;
        case Config::Basis::zyz:
            circuit.apply_operator(Op::Ry(params.theta), inst.qubits(), inst.cbits());
            break;
    }
    circuit.apply_operator(Op::Rz(params.phi), inst.qubits(), inst.cbits());
    circuit.global_phase() += params.phase;
    return true;
}

}

void euler_decomp(Circuit& circuit, Instruction const& inst, nlohmann::json const& config)
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
