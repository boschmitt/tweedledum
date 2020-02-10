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
#include <tweedledum/utils/stopwatch.hpp>
#include <typeinfo>
#include "qsp_tt.hpp"


struct qsp_tt_deps_statistics
{
  double time{0};
  uint32_t funcdep_bench_useful{0};
  uint32_t funcdep_bench_notuseful{0};
  uint32_t total_cnots{0};
  uint32_t total_rys{0};
  
}; /* qsp_tt_deps_statistics */


namespace tweedledum {

namespace detail {
//**************************************************************
void general_qg_generation(std::map <uint32_t , std::vector < std::pair < double,std::vector<uint32_t> > > >& gates,
 kitty::dynamic_truth_table tt, uint32_t var_idx_pure, std::vector<uint32_t> controls 
 ,std::map<uint32_t , std::vector<std::pair<std::string, std::vector<uint32_t>>>> dependencies , std::vector<uint32_t> const& orders)
{
    //-----co factors-------
    auto var_index = orders[var_idx_pure];
    kitty::dynamic_truth_table tt0(var_index);
    kitty::dynamic_truth_table tt1(var_index);
    tt0 = kitty::shrink_to(kitty::cofactor0(tt,var_index), tt.num_vars() - 1);
    tt1 = kitty::shrink_to(kitty::cofactor1(tt,var_index), tt.num_vars() - 1);
    //--computing probability gate---
    auto c0_ones = kitty::count_ones(tt0);
    
    auto c1_ones = kitty::count_ones(tt1);
    auto tt_ones = kitty::count_ones(tt);
    if (c0_ones!=tt_ones)
    { // == --> identity and ignore
        double angle = 2*acos(sqrt(static_cast<double> (c0_ones)/tt_ones));
        //angle *= (180/3.14159265); //in degree
        //----add probability gate----
        //gates.emplace_back("RY",angle,var_index,controls);
        auto it = dependencies.find(var_index);
        
        if(it != dependencies.end())
        {
            if(gates[var_index].size()==0)
            {  
                for(auto d = 0 ; d<dependencies[var_index].size() ; d++)
                {
                    if(dependencies[var_index][d].first == "eq") // insert cnot
                    {
                        //auto index = dependencies[var_index][d].second[0];
                        gates[var_index].emplace_back(std::pair{ M_PI,std::vector<uint32_t>{ dependencies[var_index][d].second[0]} });
                        break;
                    }
                
                    else if(dependencies[var_index][d].first == "not") // not cnot
                    {
                        //auto index = dependencies[var_index][d].second[0];
                        gates[var_index].emplace_back(std::pair{ M_PI,std::vector<uint32_t>{dependencies[var_index][d].second[0] } });
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{} });
                        break;
                    }
                    else if(dependencies[var_index][d].first == "xor")
                    {
                        for( auto d_in=0u; d_in<dependencies[var_index][d].second.size(); d_in++)
                            gates[var_index].emplace_back(std::pair{ M_PI,std::vector<uint32_t>{dependencies[var_index][d].second[d_in]} }); 
                        break;
                    }
                    else if(dependencies[var_index][d].first == "xnor")
                    {
                        for( auto d_in=0u; d_in<dependencies[var_index][d].second.size(); d_in++)
                            gates[var_index].emplace_back(std::pair{ M_PI,std::vector<uint32_t>{dependencies[var_index][d].second[d_in]} });
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{} });
                        break;
                    }
                    else if(dependencies[var_index][d].first == "and")
                    {
                        // to do --- insert nots
                        //std::for_each(dependencies[var_index][d].second.begin(), dependencies[var_index][d].second.end(), [](int32_t &el){el = abs(el)*2+1; });
                        gates[var_index].emplace_back(std::pair{ M_PI,dependencies[var_index][d].second });
                        break;
                    }
                    else if(dependencies[var_index][d].first == "nand")
                    {
                        //std::for_each(dependencies[var_index][d].second.begin(), dependencies[var_index][d].second.end(), [](int32_t &el){el = el*2+1; });
                        gates[var_index].emplace_back(std::pair{ M_PI,dependencies[var_index][d].second });
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{} });
                        break;
                    }
                    else if(dependencies[var_index][d].first == "or")
                    {
                        // to do --- insert nots
                        //std::for_each(dependencies[var_index][d].second.begin(), dependencies[var_index][d].second.end(), [](int32_t &el){el = abs(el)*2+1; });
                        gates[var_index].emplace_back(std::pair{ M_PI,dependencies[var_index][d].second });
                        for(auto d_in=0u; d_in<dependencies[var_index][d].second.size() ; d_in++)
                            gates[var_index].emplace_back(std::pair{ M_PI,std::vector<uint32_t>{dependencies[var_index][d].second[d_in]} }); 
                        break;
                    }
                    else if(dependencies[var_index][d].first == "nor")
                    {
                        //std::for_each(dependencies[var_index][d].second.begin(), dependencies[var_index][d].second.end(), [](int32_t &el){el = el*2+1; });
                        gates[var_index].emplace_back(std::pair{ M_PI,dependencies[var_index][d].second });
                        for(auto d_in=0u; d_in<dependencies[var_index][d].second.size() ; d_in++)
                            gates[var_index].emplace_back(std::pair{ M_PI,std::vector<uint32_t>{dependencies[var_index][d].second[d_in]} }); 
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{} });
                        break;
                    }
                    else if (dependencies[var_index][d].first == "and_xor")
                    {
                        // to do --- insert nots
                        /* std::for_each(dependencies[var_index][d].second.begin(), dependencies[var_index][d].second.end(), [](int32_t &el){el = abs(el)*2+1; }); */
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t> {dependencies[var_index][d].second[0],dependencies[var_index][d].second[1] } } );
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[2]} }); 
                        break;
                    }
                    else if (dependencies[var_index][d].first == "and_xnor")
                    {
                        // to do --- insert nots
                        //std::for_each(dependencies[var_index][d].second.begin(), dependencies[var_index][d].second.end(), [](int32_t &el){el = abs(el)*2+1; });
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t> {dependencies[var_index][d].second[0],dependencies[var_index][d].second[1] } } );
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[2]} }); 
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{} });
                        break;
                    }
                    else if (dependencies[var_index][d].first == "or_xor")
                    {
                        // to do --- insert nots
                        //std::for_each(dependencies[var_index][d].second.begin(), dependencies[var_index][d].second.end(), [](int32_t &el){el = abs(el)*2+1; });
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[0],dependencies[var_index][d].second[1] } } );
                        gates[var_index].emplace_back(std::pair{ M_PI,std::vector<uint32_t>{dependencies[var_index][d].second[0]} }); 
                        gates[var_index].emplace_back(std::pair{ M_PI,std::vector<uint32_t>{dependencies[var_index][d].second[1]} }); 
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[2]} }); 
                        break;   
                    }
                    else if (dependencies[var_index][d].first == "or_xnor")
                    {
                       // to do --- insert nots
                        //std::for_each(dependencies[var_index][d].second.begin(), dependencies[var_index][d].second.end(), [](int32_t &el){el = abs(el)*2+1; });
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[0],dependencies[var_index][d].second[1] } } );
                        gates[var_index].emplace_back(std::pair{ M_PI,std::vector<uint32_t>{dependencies[var_index][d].second[0]} }); 
                        gates[var_index].emplace_back(std::pair{ M_PI,std::vector<uint32_t>{dependencies[var_index][d].second[1]} }); 
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[2]} }); 
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{} });
                        break;
                    }
                }
            }   
        }
        else
            gates[var_index].emplace_back(std::pair{angle,controls});
        //std::cout<<"c0 ones: "<<c0_ones<<"tt one: "<<tt_ones<<std::endl;
        //std::cout<<"angle: "<<angle<<"control size: "<<controls.size()<<std::endl;     
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
        for(auto i=0u;i<var_idx_pure;i++)
            //gates.emplace_back("RY",M_PI/2,i,controls_new0);
            gates[ orders[i] ] .emplace_back(std::pair{M_PI/2,controls_new0});
        //--check one cofactor----
        std::vector<uint32_t> controls_new1;
        std::copy(controls.begin(), controls.end(), back_inserter(controls_new1)); 
        auto ctrl1 = var_index*2 + 0; //positive control: /2 ---> index %2 ---> sign
        controls_new1.emplace_back(ctrl1);
        if(c1_allone){
            //---add H gates---
            for(auto i=0u;i<var_idx_pure;i++)
                //gates.emplace_back("RY",M_PI/2,i,controls_new1);
                gates[ orders[i] ].emplace_back(std::pair{M_PI/2,controls_new1});

        }
        else if(c1_allzero){
            return;
        }
        else{//some 1 some 0
            general_qg_generation(gates,tt1,var_idx_pure-1,controls_new1 , dependencies , orders);
        }
    }
    else if(c0_allzero){
        //--check one cofactor----
        std::vector<uint32_t> controls_new1;
        std::copy(controls.begin(), controls.end(), back_inserter(controls_new1)); 
        auto ctrl1 = var_index*2 + 0; //positive control: /2 ---> index %2 ---> sign
        controls_new1.emplace_back(ctrl1);
        if(c1_allone){
            //---add H gates---
            for(auto i=0u;i<var_idx_pure;i++)
                //gates.emplace_back("RY",M_PI/2,i,controls_new1);
                gates[ orders[i] ].emplace_back(std::pair{M_PI/2,controls_new1});

        }
        else if(c1_allzero){
            return;
        }
        else{//some 1 some 0
            general_qg_generation(gates,tt1,var_idx_pure-1,controls_new1 , dependencies , orders);
        }
    }
    else{//some 0 some 1 for c0
        
        std::vector<uint32_t> controls_new1;
        std::copy(controls.begin(), controls.end(), back_inserter(controls_new1)); 
        auto ctrl1 = var_index*2 + 0; //positive control: /2 ---> index %2 ---> sign
        controls_new1.emplace_back(ctrl1);
        if(c1_allone){
            general_qg_generation(gates,tt0,var_idx_pure-1,controls_new0 , dependencies , orders);
            //---add H gates---
            for(auto i=0u;i<var_idx_pure;i++)
                //gates.emplace_back("RY",M_PI/2,i,controls_new1);
                gates[ orders[i] ].emplace_back(std::pair{M_PI/2,controls_new1});

        }
        else if(c1_allzero){
            general_qg_generation(gates,tt0,var_idx_pure-1,controls_new0 , dependencies , orders);
            //return;
        }
        else{//some 1 some 0
            general_qg_generation(gates,tt0,var_idx_pure-1,controls_new0 , dependencies , orders);
            general_qg_generation(gates,tt1,var_idx_pure-1,controls_new1 , dependencies , orders);
        }
    }

}

template<typename Network>
void qsp_ownfunction( Network& net, 
const kitty::dynamic_truth_table tt, 
std::map<uint32_t, std::vector<std::pair<std::string, std::vector<uint32_t>>>> const dependencies, 
qsp_tt_deps_statistics& stats , std::vector<uint32_t> const& orders)
{
    std::map <uint32_t , std::vector < std::pair < double,std::vector<uint32_t> > > > gates;  
    auto tt_vars = tt.num_vars();
    
    auto var_idx = tt_vars-1;
    std::vector<uint32_t> cs;
    
    general_qg_generation(gates, tt, var_idx, cs, dependencies , orders);
    
    auto total_rys = 0;
    auto total_cnots = 0;

    bool sig;
    auto n_reduc = 0; // lines that always are zero or one and so we dont need to prepare them

    std::vector< std::pair<uint32_t,uint32_t> > gates_num(tt_vars); //rys,cnots
    for(int32_t i=gates.size()-1; i>=0;i--)
    {
        if(gates[ orders[i] ].size()==0)
        {
            gates_num[ orders[i] ] = std::make_pair(0,0); // number of rys and cnots
            n_reduc++;
            continue;
        }

        auto rys = 0;
        auto cnots = 0;
        sig = 1;
        
        for(auto j=0u; j< gates[ orders[i] ].size(); j++)
        {
            if(gates[ orders[i] ][j].second.size()==((tt.num_vars()-i-1)-n_reduc) &&
            gates[ orders[i] ][j].second.size()!=0) // number of controls is max or not?
            {
                sig = 0;
            }
        
            auto cs = gates[ orders[i] ][j].second.size();
            if(cs==0)
                rys += 1;
            else if (cs==1 && (std::abs(gates[orders[i]][j].first-M_PI)<0.1))
                cnots += 1;
            else
            {
                rys += pow(2,cs);
                cnots += pow(2,cs);
            }
            
        }
        if (sig==0) // we have max number of controls
        {
            if(i==(tt.num_vars()-1-n_reduc)) // first line for preparation
            {
                cnots = 0;
                rys = 1;
            }
            else if(gates[orders[i]].size()==1 && (std::abs(gates[orders[i]][0].first-M_PI)<0.1) && gates[orders[i]][0].second.size()==1) // second line for preparation
            {               
                cnots = 1;
                rys = 0;
            }
            else // other lines with more than one control
            {
                rys = pow(2,((tt.num_vars()-i-1)-n_reduc));
                cnots = pow(2,((tt.num_vars()-i-1)-n_reduc)); 
            }
            
        }  
        
        gates_num[orders[i]] = std::make_pair(rys,cnots);
            //out_file<<"i:"<<i<<"  "<<rys<<"  "<<cnots<<"  ";

        total_rys += rys;
        total_cnots += cnots;  
    }
    
    // std::cout<<"n_reduc: "<<n_reduc<<std::endl;
    stats.total_cnots += total_cnots;
    stats.total_rys += total_rys;

    //stats.reduction += n_reduc;
    
    if(total_cnots < (pow(2,tt_vars-n_reduc)-2) && dependencies.size()>0) 
    {       
        ++stats.funcdep_bench_useful;
    }
   
    else if(total_cnots >= (pow(2,tt_vars-n_reduc)-2) && dependencies.size()>0)
    {
        ++stats.funcdep_bench_notuseful;
    }
    
    //detail::extract_multiplex_gates(net,tt_vars,gates);  
}

template<typename Network>
void qsp_allone_first(Network& net, 
const kitty::dynamic_truth_table tt, 
std::map<uint32_t, std::vector<std::pair<std::string, std::vector<uint32_t>>>> const dependencies, 
qsp_tt_deps_statistics& stats , std::vector<uint32_t> const& orders)
{
    std::map <uint32_t , std::vector < std::pair < double,std::vector<uint32_t> > > > gates;
    auto tt_vars = tt.num_vars();
    
    auto ones = kitty::count_ones(tt);

    auto tt_new = kitty::create<kitty::dynamic_truth_table>(tt_vars);
    for(auto i=0u;i<ones;i++)
        kitty::set_bit(tt_new,i);

    auto var_idx = tt_vars-1;
    std::vector<uint32_t> cs;
    general_qg_generation(gates, tt, var_idx, cs, dependencies , orders);
    //detail::control_line_cancelling(gates,tt_vars);

    //qc_generation(net,gates);
    
    // std::vector<qubit_id> qubits(tt_vars);
	// std::iota(qubits.begin(), qubits.end(), 0u);
    // std::vector<uint32_t> perm;
    
    // for(auto i=0u;i<tt_str.size();i++)
    //     if(tt_str[i]=='1')
    //         perm.emplace_back(i);
    // for(auto i=ones;i<tt_str.size();i++)
    //     perm.emplace_back(i);

    // for(auto i=0u;i<perm.size();i++)
    // std::cout<<perm[i]<<" ";

    // detail::tbs_unidirectional(net, qubits, perm,ones);
                          
}

} //end detail


template<class Network>
void qsp_tt_dependencies(Network& network, const kitty::dynamic_truth_table tt, 
std::map<uint32_t, std::vector<std::pair<std::string, std::vector<uint32_t>>>> const dependencies, 
 std::vector<uint32_t> const& orders, qsp_tt_deps_statistics& stats, qsp_params params = {} )
{
  
    const uint32_t num_qubits = tt.num_vars();
    for (auto i = 0u; i < num_qubits; ++i) 
    {
      network.add_qubit();
    }
	
    stopwatch<>::duration time_traversal{0};
    switch (params.strategy) 
    {
    	case qsp_params::strategy::allone_first:
    		detail::qsp_allone_first(network, tt , dependencies, stats, orders);
    		break;
    	case qsp_params::strategy::ownfunction:
            detail::qsp_ownfunction( network, tt, dependencies, stats , orders);
            break;
    }
    stopwatch t( time_traversal );
      
    stats.time = to_seconds( time_traversal );
}

template<class Network>
void qsp_tt_dependencies( Network& network, const kitty::dynamic_truth_table tt, 
std::map<uint32_t, std::vector<std::pair<std::string, std::vector<uint32_t>>>> const dependencies, 
qsp_tt_deps_statistics& stats, qsp_params params = {})
{
    qsp_tt_dependencies( network, tt, dependencies, detail::initialize_orders( tt.num_vars() ), stats, params);
}

} // namespace tweedledum end
