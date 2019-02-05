#pragma once

#include "../../gates/gate_set.hpp"
#include "../../gates/gate_base.hpp"
#include "../../networks/qubit.hpp"
#include "../generic/rewrite.hpp"

#include <array>
#include <iostream>
#include <vector>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/esop.hpp>
#include <kitty/operations.hpp>
#include <kitty/print.hpp>


namespace tweedledum {
namespace detail {

template<typename Network>
void name(){

}


} // namespace detail
//**************************************************************
template<typename Network>
void qs_basic(Network const& net, const std::string &tt_str)
{
    auto tt_vars = int(log2(tt_str.size()));
    
    kitty::dynamic_truth_table tt(tt_vars);
    kitty::create_from_binary_string( tt, tt_str);
    std::cout<<"vars tt: "<<tt.num_vars()<<std::endl;
    kitty::print_binary(tt);

    //-----co factors-------
    kitty::dynamic_truth_table tt0(tt_vars-1);
    kitty::dynamic_truth_table tt1(tt_vars-1);

    tt0 = kitty::shrink_to(kitty::cofactor0(tt,2), tt.num_vars() - 1);
    std::cout<<"\n sss: "<<tt0.num_vars()<<std::endl;
    kitty::print_binary(tt0);
    
}

} // namespace tweedledum
