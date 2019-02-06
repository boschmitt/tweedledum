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
#include <vector>


namespace tweedledum {
namespace detail {

template<typename Network>
void name(){

}


} // namespace detail end
//**************************************************************

void general_qgs_generation(std::vector < std::tuple < std::string,double,uint32_t,std::vector<uint32_t> > >& gates, kitty::dynamic_truth_table tt, uint32_t var_index, std::vector<uint32_t> controls)
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
    std::cout<<"c0_ones: "<< pow(2, tt0.num_vars()) <<std::endl;
    auto c1_ones = kitty::count_ones(tt1);
    auto tt_ones = kitty::count_ones(tt);
    if (c0_ones!=tt_ones){ // == --> identity and ignore
    std::cout<<"idx: "<<var_index<<std::endl;
        double angle = 2*acos(sqrt(static_cast<double> (c0_ones)/tt_ones));
        angle *= (180/3.14159265); //in degree
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
        std::cout<<"c0_allone\n";
        //---add H gates---
        for(auto i=0u;i<var_index;i++)
            gates.emplace_back("H",90,i,controls_new0);
        //--check one cofactor----
        std::vector<uint32_t> controls_new1;
        std::copy(controls.begin(), controls.end(), back_inserter(controls_new1)); 
        auto ctrl1 = var_index*2 + 0; //negetive control: /2 ---> index %2 ---> sign
        controls_new1.emplace_back(ctrl1);
        if(c1_allone){
            kitty::print_binary(tt1);
        std::cout<<"c1 allone\n";
            //---add H gates---
            for(auto i=0u;i<var_index;i++)
                gates.emplace_back("H",90,i,controls_new1);

        }
        else if(c1_allzero){
            return;
        }
        else{//some 1 some 0
        kitty::print_binary(tt1);
        std::cout<<"\n";
            general_qgs_generation(gates,tt1,var_index-1,controls_new1);
        }
    }
    else if(c0_allzero){
        return;
    }
    else{//some 0 some 1 for c0
        general_qgs_generation(gates,tt0,var_index-1,controls_new0);
    }

}

template<typename Network>
void qc_generation(std::vector < std::tuple < std::string,double,uint32_t,std::vector<uint32_t> > > gates, Network const& net){
    for(const auto [name,angle,target_id,controls]: gates){
        if(name=="RY"){
            
        }
    }

}

template<typename Network>
void qs_basic(Network const& net, const std::string &tt_str)
{

    std::vector < std::tuple < std::string,double,uint32_t,std::vector<uint32_t> > > gates; // gate name, angle, target_id, controls:id and sign /2 and %2

    auto tt_vars = int(log2(tt_str.size()));
    
    kitty::dynamic_truth_table tt(tt_vars);
    kitty::create_from_binary_string( tt, tt_str);
    std::cout<<"vars tt: "<<tt.num_vars()<<std::endl;
    kitty::print_binary(tt);

    //-----co factors-------
    auto var_idx = tt_vars-1;
    kitty::dynamic_truth_table tt0(tt_vars-1);
    kitty::dynamic_truth_table tt1(tt_vars-1);

    tt0 = kitty::shrink_to(kitty::cofactor0(tt,var_idx), tt.num_vars() - 1);
    std::cout<<"\nsss: "<<tt0.num_vars()<<std::endl;
    kitty::print_binary(tt0);
    std::cout<<std::endl;
    std::vector<uint32_t> cs;
    general_qgs_generation(gates,tt,var_idx,cs);
    for(auto i=0u;i<gates.size();i++){
        std::cout<<"gates:\n";
        std::cout<<std::get<0> (gates[i]) <<std::endl;
        std::cout<<std::get<1> (gates[i]) <<std::endl;
        std::cout<<std::get<2> (gates[i]) <<std::endl;
        for(auto j=0;j< (std::get<3> (gates[i])).size();j++)
            std::cout<<(std::get<3> (gates[i])) [j] <<std::endl;
    }

    qc_generation(gates,net);
    
}

} // namespace tweedledum end
