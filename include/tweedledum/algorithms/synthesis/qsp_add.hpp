#pragma once

#include "../../gates/gate_set.hpp"
#include "../../gates/gate_base.hpp"
#include "../../gates/mcst_gate.hpp"
#include "../../gates/mcmt_gate.hpp"
#include "../../networks/qubit.hpp"
#include "../generic/rewrite.hpp"
#include "linear_synth.hpp"
#include "tbs.hpp"
#include <array>
#include <iostream>
#include <vector>
#include <map>
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

void parse_add(std::unordered_set<DdNode*>& visited, 
std::vector<std::map<DdNode *,uint32_t>>  &node_ones, 
DdNode *f, uint32_t num_vars)
{
    auto current = f;

    if ( visited.count( current ) ) 
        return;
    if ( Cudd_IsConstant( current ) ) 
        return;
    
    std::cout<<"yes add\n";
    parse_add( visited, node_ones, cuddE( current ), num_vars );
    parse_add( visited, node_ones , cuddT( current ), num_vars );

    visited.insert( current );
    uint32_t Eones = 0;
    uint32_t Tones = 0;

    if( Cudd_IsConstant(cuddT(current)))
    {
        if(Cudd_V(cuddT(current)))
        {
            auto temp_pow = num_vars - current->index - 1;
            Tones = pow(2,temp_pow) * 1 ;  
        }
    }
    else{
        auto temp_pow = cuddT(current)->index - 1 - current->index;
        auto Tvalue = 0;
        Tvalue = node_ones[cuddT(current)->index].find(cuddT(current))->second;
        Tones = pow(2,temp_pow) * Tvalue;
    }
    
    if( Cudd_IsConstant(cuddE(current)))
    {
        if (Cudd_V(cuddE(current)))
        {
            auto temp_pow = num_vars - current->index - 1; 
            Eones = pow(2,temp_pow) * 1 ;
        }
        
    }
    else{
        auto temp_pow = cuddE(current)->index - 1 - current->index;
        auto max_ones = pow(2,num_vars- cuddE(current)->index);
        auto Evalue = 0;
        Evalue = node_ones[cuddE(current)->index].find(cuddE(current))->second;
        Eones = pow(2,temp_pow) * Evalue;
    }
    std::cout<<"index: "<<current->index<<"  ";
    std::cout<<"weight: "<< Tones+Eones <<std::endl;
    node_ones[current->index].insert({current,Tones+Eones});
} /* end of parse_add function */

void extract_gates(std::unordered_set<DdNode*>& visited,
std::vector<std::map<DdNode *,uint32_t >> node_ones,
std::vector< std::vector<std::pair<double , std::vector<int32_t>>> > &gates, 
std::vector<int32_t> &controls,
DdNode *f, uint32_t num_vars)
{
    auto current = f;
    if ( visited.count(current) ) 
        return;
    if ( Cudd_IsConstant(current) ) 
        return;

    visited.insert(current);
    
    double ep = 0;
    
    double dp = node_ones[current->index].find(current)->second;
    
    if( Cudd_IsConstant(cuddE(current)) && Cudd_V(cuddE(current)) )
    {
        ep = pow(2 , num_vars-1-current->index);
    }
    else
    {
        ep = node_ones[cuddE(current)->index].find(cuddE(current))->second;
    }
    
    double p = ep/dp;
    std::cout<<"index: "<<current->index<<" p: "<<p<<std::endl;
    for(auto i=0;i<controls.size();i++)
    std::cout<<controls[i]<<"  ";
    std::cout<<std::endl;

    gates[current->index].emplace_back(p,controls);
    auto Econtrols = controls;
    auto Tcontrols = controls;
    Econtrols.emplace_back(-(current->index+1));
    Tcontrols.emplace_back(current->index+1);
    
    auto upE = Cudd_IsConstant(cuddE(current)) ? num_vars : cuddE(current)->index;
    auto upT = Cudd_IsConstant(cuddT(current)) ? num_vars : cuddT(current)->index;
    for(auto i = current->index+1 ; i < upE ; i++)
    {
        gates[i].emplace_back(1.0/2,Econtrols);
    }
    for(auto i = current->index+1 ; i < upT ; i++)
    {
        gates[i].emplace_back(1.0/2,Tcontrols);
    }

    extract_gates( visited , node_ones , gates , Econtrols , cuddE(current) , num_vars );
    extract_gates( visited , node_ones , gates , Tcontrols , cuddT(current) , num_vars );

} /* end of extract_gates function */

} // namespace detail end
//**************************************************************

template<class Network>
void add_gates_to_network(Network &net , 
std::vector< std::vector<std::pair<double , std::vector<int32_t>>> > gates)
{
    
    for(auto i=0u ; i<gates.size() ; i++)
    {
        for(auto j=0u ; j<gates[i].size() ; j++)
        {
            std::vector<qubit_id> controls;
            for(auto k=0u ; k<gates[i][j].second.size() ; k++)
            {
                if(gates[i][j].second[k] < 0)
                {
                    controls.emplace_back(-gates[i][j].second[k] - 1);
                    controls[k].complement();
                }
                else
                {
                    controls.emplace_back(gates[i][j].second[k] - 1);
                }
                
                
            }
            std::vector<qubit_id> t;
            t.emplace_back(i);
            net.add_gate(gate_base(gate_set::rotation_y, gates[i][j].first) , controls , t);
            std::cout<<"angle: "<<gates[i][j].first<<std::endl;
        }
    }
    
}


template<class Network>
void qsp_add(Network& network)//, const std::string &tt_str, qsp_params params = {})
{
   
   /*
    Network initialization
    */
    //assert(tt_str.size() <= pow(2,6));
    const uint32_t num_qubits = 4; //std::log2(tt_str.size());
    for (auto i = 0u; i < num_qubits; ++i) {
		network.add_qubit();
	}
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
    std::vector<std::map <DdNode * , uint32_t > > node_ones(mgr.ReadSize());
    std::vector<std::pair<uint32_t,uint32_t>> ones_frac;
    std::unordered_set<DdNode*> visited;

    detail::parse_add( visited , node_ones , f_add , mgr.ReadSize());
    std::unordered_set<DdNode*> visited1;
    std::vector< std::vector<std::pair<double , std::vector<int32_t>>> > gates (mgr.ReadSize());
    std::vector<int32_t> controls;
    detail::extract_gates(visited1 , node_ones , gates , controls , f_add , mgr.ReadSize());
    
    add_gates_to_network(network,gates);

char x = 'A'+1;
    std::cout<<x<<std::endl;
	
}

} /* namespace tweedledum end */
