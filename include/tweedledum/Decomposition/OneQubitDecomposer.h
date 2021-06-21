/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../IR/Instruction.h"

#include <nlohmann/json.hpp>

namespace tweedledum {

class OneQubitDecomposer {
public:
    struct Config {
        enum class Basis
        {
            px, // Op::P, Op::Rx
            psx, // Op::P, Op::SX
            xyx, // Op::Rx, Op::Ry
            zsx, // Op::Rz, Op::SX
            zsxx, // Op::Rz, Op::X, Op::SX
            zxz, // Op:Rz, Op::Rx
            zyz // Op::Rz, Op::Ry
        };
        Basis basis;
        bool simplify;
        double atol;

        Config(nlohmann::json const& config);
    };
    using Basis = Config::Basis;
    Config config;

    OneQubitDecomposer(nlohmann::json const& config = {})
        : config(config)
    {}

    bool decompose(Circuit& circuit, Instruction const& inst);

private:
    struct Params {
        double theta;
        double lambda;
        double phi;
        double phase;
    };

    // Bring the 'difference' between two angles into [-pi; pi].
    static inline double normalize_npi_pi(double angle, double atol = 0.0);

    static inline Params zyz_params(UMatrix const& matrix);
    static inline Params zxz_params(UMatrix const& matrix);
    static inline Params px_params(UMatrix const& matrix);
    static inline Params xyx_params(UMatrix const& matrix);

    static inline double add_rx(
      Circuit& circuit, Instruction const& inst, double angle, double atol);

    static inline void add_ry(
      Circuit& circuit, Instruction const& inst, double angle, double atol);

    static inline double add_rz(
      Circuit& circuit, Instruction const& inst, double angle, double atol);

    static inline double add_p(
      Circuit& circuit, Instruction const& inst, double angle);

    static inline double add_sx(Circuit& circuit, Instruction const& inst);

    static inline double add_rx_pi_2(Circuit& circuit, Instruction const& inst);

    template<typename FnParams, typename FnXY, typename FnXZ>
    inline bool circuit_xz_xy(Circuit& circuit, Instruction const& inst,
      FnParams&& compute_params, FnXZ&& add_rx_rz, FnXY&& add_rx_ry);

    template<typename FnParams, typename FnP, typename FnX>
    inline bool circuit_pz_xsx(Circuit& circuit, Instruction const& inst,
      FnParams&& compute_params, FnP&& add_phase, FnX&& add_x);
};

} // namespace tweedledum
