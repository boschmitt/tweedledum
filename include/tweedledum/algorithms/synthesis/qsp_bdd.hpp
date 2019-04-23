#pragma once

#include "../../gates/gate_set.hpp"
#include "../../gates/gate_base.hpp"
#include "../../networks/qubit.hpp"
#include "../generic/rewrite.hpp"
#include "linear_synth.hpp"
#include "tbs.hpp"
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
#include <math.h>
#include <cudd/cudd.h>
#include <cudd/cuddInt.h>
#include <cplusplus/cuddObj.hh>

#include <typeinfo>

namespace tweedledum {
namespace detail {




} // namespace detail end
//**************************************************************


void parse_bdd(std::unordered_set<DdNode*>& visited, 
std::vector<std::vector<std::tuple<DdNode *,uint32_t>>>  &node_ones, 
DdNode *f, uint32_t num_vars)
{
    auto current = Cudd_Regular (f);

    if ( visited.count( current ) ) 
        return;
    if ( Cudd_IsConstant( current ) ) 
        return;
    
    parse_bdd( visited, node_ones, cuddE( current ), num_vars );
    parse_bdd( visited, node_ones , cuddT( current ), num_vars );

    std::cout<<"index: "<<current->index<<std::endl;
    visited.insert( current );
    uint32_t Eones = 0;
    uint32_t Tones = 0;

    if( Cudd_IsConstant(cuddT(current)) ){
        auto temp_pow = num_vars - current->index - 1;
        Tones = pow(2,temp_pow) * 1 ;  
    }
    else{
        auto temp_pow = cuddT(current)->index - 1 - current->index;
        auto Tvalue = 0;
        for(auto i=0u;i<node_ones[cuddT(current)->index].size();i++)
            if(std::get<0> (node_ones[cuddT(current)->index][i]) == cuddT(current))
                Tvalue = std::get<1> (node_ones[cuddT(current)->index][i]);
        Tones = pow(2,temp_pow) * Tvalue;
    }
    auto cuddEregular = Cudd_Regular(cuddE(current));
    if( Cudd_IsConstant(cuddE(current)) ){
        auto temp_pow = num_vars - current->index - 1; 
        Eones = pow(2,temp_pow) * (1-Cudd_IsComplement(cuddE(current))) ;
    }
    else{
        auto temp_pow = cuddEregular->index - 1 - current->index;
        auto max_ones = pow(2,num_vars- cuddEregular->index);
        auto Evalue = 0;
       // std::cout<<"---: "<<Cudd_Regular(cuddE(current))->index<<std::endl;
        //std::cout<<"E else: "<<node_ones[cuddEregular->index].size()<<std::endl;
        for(auto i=0u;i<node_ones[cuddEregular->index].size();i++){
            //std::cout<<"E else for: "<<std::get<0> (node_ones[cuddEregular->index][i])->index <<std::endl;
            if(std::get<0> (node_ones[cuddEregular->index][i]) == cuddEregular){
                Evalue = std::get<1> (node_ones[cuddEregular->index][i]);
                //std::cout<<"evalue: "<<Evalue<<std::endl;
            }
        }
        //std::cout<<"comp: "<<Cudd_IsComplement(cuddE(current))<<std::endl;
        //std::cout<<"e value1: "<<Evalue<<std::endl;
        Evalue = Cudd_IsComplement(cuddE(current)) ? (max_ones-Evalue) : (Evalue);
        //std::cout<<"e value: "<<Evalue<<std::endl;
        Eones = pow(2,temp_pow) * Evalue;
    }

 //std::cout<<"weight T: "<< Tones <<std::endl;
  //std::cout<<"weight E: "<< Eones <<std::endl;
    std::cout<<"weight: "<< Tones+Eones <<std::endl;
    //std::cout<<"ref: "<< current->ref <<std::endl;
    node_ones[current->index].emplace_back(current,Tones+Eones);

}


template<class Network>
void qsp_bdd(Network& network)//, const std::string &tt_str, qsp_params params = {})
{
    /*
    Network initialization
    */
    // assert(tt_str.size() <= pow(2,6));
    // const uint32_t num_qubits = std::log2(tt_str.size());
    // for (auto i = 0u; i < num_qubits; ++i) {
	// 	network.add_qubit();
	// }
	/*
    Create BDD
    */
    Cudd mgr;

    auto d = mgr.bddVar(); //MSB
    auto c = mgr.bddVar();
    auto b = mgr.bddVar();
    auto a = mgr.bddVar(); //LSB
    //auto f_bdd = (a & b & c & d) | (!a & !b & !c & !d) ;
    //1100 1010 1111 1110
    auto f_bdd = 
        (!d & !c & !b & !a) | 
        (!d & !c & !b & a) | 
        (!d & c & !b & !a) | 
        (!d & c & b & !a) | 
        (d & !c & !b & !a) |
        (d & !c & !b & a) | 
        (d & !c & b & !a) | 
        (d & !c & b & a) | 
        (d & c & !b & !a) | 
        (d & c & !b & a) | 
        (d & c & b & !a);

    auto f_add = Cudd_BddToAdd(mgr.getManager(),f_bdd.getNode());
    

    //mgr.DumpDot(  f_add );
    std::vector<std::vector<std::tuple<DdNode *,uint32_t>>> node_ones(mgr.ReadSize());
    std::unordered_set<DdNode*> visited;
    parse_bdd( visited, node_ones, f_add,mgr.ReadSize());
	
}

} /* namespace tweedledum end */
