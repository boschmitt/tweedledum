#pragma once

#include "../../gates/gate_set.hpp"
#include "../../gates/gate_base.hpp"
#include "../../networks/qubit.hpp"
#include "../../utils/parity_terms.hpp"
#include "../generic/rewrite.hpp"
#include "../synthesis/dbs.hpp"
#include "../synthesis/linear_synth.hpp"
#include <array>
#include <iostream>
#include <vector>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>

#include <kitty/operations.hpp>
#include <kitty/print.hpp>
#include <kitty/spectral.hpp>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/esop.hpp>
#include <kitty/operations.hpp>
#include <kitty/print.hpp>

namespace tweedledum {

template<class Network>
void decomposition_mcz(Network& net,  std::vector<qubit_id> const& q_map)//changed by fereshte
{
    
    unsigned nlines = q_map.size();
    auto tt = kitty::create<kitty::dynamic_truth_table>(nlines-1);
    kitty::set_bit(tt,pow(2,nlines-1)-1);
        
    const auto num_controls = tt.num_vars();
    //assert(qubit_map.size() == num_controls + 1);
    
    auto g = kitty::extend_to(tt, num_controls + 1);
    auto xt = g.construct();
    kitty::create_nth_var(xt, num_controls);
    g &= xt;
    std::cout<<"print tt2:\n";
    kitty::print_binary(tt);
    std::cout<<std::endl;

    parity_terms parities;
    const float nom = M_PI / (1 << g.num_vars());
    const auto spectrum = kitty::rademacher_walsh_spectrum(g);
    for (auto i = 1u; i < spectrum.size(); ++i) {
        if (spectrum[i] == 0) {
            continue;
        }
        parities.add_term(i, nom * spectrum[i]);
        std::cout<<i<<std::endl;
    }
    
    detail::linear_synth_gray(net, q_map, parities);

        
}

}//end namespace tweedledum
