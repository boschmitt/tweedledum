#pragma once

#include "../../gates/gate_set.hpp"
#include "../../gates/gate_base.hpp"
#include "../../networks/qubit.hpp"
#include "../generic/rewrite.hpp"
#include "linear_synth.hpp"

#include <array>
#include <iostream>
#include <vector>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/esop.hpp>
#include <kitty/operations.hpp>
#include <kitty/print.hpp>
#include <kitty/kitty.hpp>
#include <vector>


namespace tweedledum {
namespace detail {
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

} // namespace detail end
//**************************************************************

void general_qg_generation(std::vector < std::tuple < std::string,double,uint32_t,std::vector<uint32_t> > >& gates, kitty::dynamic_truth_table tt, uint32_t var_index, std::vector<uint32_t> controls)
{
    if(var_index==-1)
        return;
    //-----co factors-------
    kitty::dynamic_truth_table tt0(var_index);
    kitty::dynamic_truth_table tt1(var_index);
    tt0 = kitty::shrink_to(kitty::cofactor0(tt,var_index), tt.num_vars() - 1);
    tt1 = kitty::shrink_to(kitty::cofactor1(tt,var_index), tt.num_vars() - 1);
    //--computing probability gate---
    auto c0_ones = kitty::count_ones(tt0);
    
    auto c1_ones = kitty::count_ones(tt1);
    auto tt_ones = kitty::count_ones(tt);
    if (c0_ones!=tt_ones){ // == --> identity and ignore
        double angle = 2*acos(sqrt(static_cast<double> (c0_ones)/tt_ones));
        //angle *= (180/3.14159265); //in degree
        //----add probability gate----
        gates.emplace_back("RY",angle,var_index,controls);
        std::cout<<"angle: "<<angle<<std::endl;
    }
    //-----qc of cofactors-------
    //---check state--- 
    auto c0_allone = (c0_ones==pow(2, tt0.num_vars())) ? true : false ;
    auto c0_allzero = (c0_ones==0) ? true : false ;
    auto c1_allone = (c1_ones==pow(2, tt1.num_vars())) ? true : false ;
    auto c1_allzero = (c1_ones==0) ? true : false ;

    std::vector<uint32_t> controls_new0;
    std::copy(controls.begin(), controls.end(), back_inserter(controls_new0)); 
    auto ctrl0 = var_index*2 + 1; //negetive control: /2 ---> index %2 ---> sign
    controls_new0.emplace_back(ctrl0);
    if (c0_allone){
        
        //---add H gates---
        for(auto i=0u;i<var_index;i++)
            gates.emplace_back("H",90,i,controls_new0);
        //--check one cofactor----
        std::vector<uint32_t> controls_new1;
        std::copy(controls.begin(), controls.end(), back_inserter(controls_new1)); 
        auto ctrl1 = var_index*2 + 0; //negetive control: /2 ---> index %2 ---> sign
        controls_new1.emplace_back(ctrl1);
        if(c1_allone){
            //---add H gates---
            for(auto i=0u;i<var_index;i++)
                gates.emplace_back("H",90,i,controls_new1);

        }
        else if(c1_allzero){
            return;
        }
        else{//some 1 some 0
            general_qg_generation(gates,tt1,var_index-1,controls_new1);
        }
    }
    else if(c0_allzero){
        return;
    }
    else{//some 0 some 1 for c0
        general_qg_generation(gates,tt0,var_index-1,controls_new0);
    }

}

template<typename Network>
void qc_generation(Network & net, std::vector < std::tuple < std::string,double,uint32_t,std::vector<uint32_t> > > gates){
    for(const auto [name,angle,target_id,controls]: gates){
        if(name=="RY"){
            std::cout<<"RY: "<<target_id<<std::endl;
            if(controls.size()==0){
                
                auto temp = gate_base(gate_set::rotation_y, angle);
                std::cout<<"RY--: "<<target_id<<std::endl;
                std::cout<<"RY--: "<<net.num_qubits()<<std::endl;
                net.add_gate(temp, target_id);
                std::cout<<"RY: "<<target_id<<std::endl;
            }
            else{//we have multi control probability gate
            std::vector<qubit_id> q_map;
                net.add_gate(gate_base(gate_set::rotation_y, angle/2), target_id);
                for(const auto ctrl:controls){
                    if(ctrl%2 == 1)//negative control
                        net.add_gate(gate::pauli_x, ctrl/2);
                    q_map.emplace_back(ctrl/2);
                }
                q_map.emplace_back(target_id);
                detail::decomposition_mcz(net,q_map);
                for(const auto ctrl:controls){
                    if(ctrl%2 == 1)//negative control
                        net.add_gate(gate::pauli_x, ctrl/2);
                }
                net.add_gate(gate_base(gate_set::rotation_y, -angle/2), target_id);

            }//end multi control
        }//end RY
        else if(name=="H"){
            std::cout<<"H: "<<target_id<<std::endl;
            if(controls.size()==0)
                net.add_gate(gate::hadamard, target_id);
            else{//we have multi control probability gate
                std::vector<qubit_id> q_map;
                net.add_gate(gate_base(gate_set::rotation_y, 3.14159265/4), target_id);
                for(const auto ctrl:controls){
                    if(ctrl%2 == 1)//negative control
                        net.add_gate(gate::pauli_x, ctrl/2);
                    q_map.emplace_back(ctrl/2);
                }
                q_map.emplace_back(target_id);
                detail::decomposition_mcz(net,q_map);
                for(const auto ctrl:controls){
                    if(ctrl%2 == 1)//negative control
                        net.add_gate(gate::pauli_x, ctrl/2);
                }
                net.add_gate(gate_base(gate_set::rotation_y, -3.14159265/4), target_id);
            }//end multi control
        }//end H

    }// end gates

}

template<typename Network>
void qs_basic(Network & net, const std::string &tt_str)
{
    const uint32_t num_qubits = std::log2(tt_str.size());
    for (auto i = 0u; i < num_qubits; ++i) {
		net.add_qubit();
	}

    std::vector < std::tuple < std::string,double,uint32_t,std::vector<uint32_t> > > gates; // gate name, angle, target_id, controls:id and sign /2 and %2

    auto tt_vars = int(log2(tt_str.size()));
    
    kitty::dynamic_truth_table tt(tt_vars);
    kitty::create_from_binary_string( tt, tt_str);

    //-----co factors-------
    auto var_idx = tt_vars-1;
    kitty::dynamic_truth_table tt0(tt_vars-1);
    kitty::dynamic_truth_table tt1(tt_vars-1);

    tt0 = kitty::shrink_to(kitty::cofactor0(tt,var_idx), tt.num_vars() - 1);
    std::vector<uint32_t> cs;
    general_qg_generation(gates,tt,var_idx,cs);
    for(auto i=0u;i<gates.size();i++){
        std::cout<<"gates:\n";
        std::cout<<std::get<0> (gates[i]) <<std::endl;
        std::cout<<std::get<1> (gates[i]) <<std::endl;
        std::cout<<std::get<2> (gates[i]) <<std::endl;
        for(auto j=0;j< (std::get<3> (gates[i])).size();j++)
            std::cout<<(std::get<3> (gates[i])) [j] <<std::endl;
    }

    qc_generation(net,gates);
    
}

} // namespace tweedledum end
