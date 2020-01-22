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

uint32_t all_bench = 0;
uint32_t funcdep_bench_useful = 0;
uint32_t funcdep_bench_notuseful=0;

struct qsp_tt_statistics
{
  double time{0};
  uint32_t reduction{0};
  uint32_t total_cnots{0};
  uint32_t total_rys{0};
  
}; /* qsp_tt_statistics */


namespace tweedledum {
namespace detail {
template<class Network>
void decomposition_mcz(Network& net,  std::vector<qubit_id> const& q_map, kitty::dynamic_truth_table tt)//changed by fereshte
{
    
    //unsigned nlines = q_map.size();
    //auto tt = kitty::create<kitty::dynamic_truth_table>(nlines-1);
    //kitty::set_bit(tt,pow(2,nlines-1)-1);
    //kitty::set_bit(tt,5);
        
    const auto num_controls = tt.num_vars();
    //assert(qubit_map.size() == num_controls + 1);
    
    auto g = kitty::extend_to(tt, num_controls + 1);
    auto xt = g.construct();
    kitty::create_nth_var(xt, num_controls);
    g &= xt;
    

    parity_terms parities;
    const float nom = M_PI / (1 << g.num_vars());
    const auto spectrum = kitty::rademacher_walsh_spectrum(g);
    for (auto i = 1u; i < spectrum.size(); ++i) {
        if (spectrum[i] == 0) {
            continue;
        }
        parities.add_term(i, nom * spectrum[i]);
        
    }
    
    detail::linear_synth_gray(net, q_map, parities);

        
}

std::vector<double> gauss(std::vector< std::vector<double> > A) 
{
    auto n = A.size();
    std::cout<<"m size: "<<n<<std::endl;
    for (auto i=0u; i<n; i++) {
        // Search for maximum in this column
        double maxEl = abs(A[i][i]);
        auto maxRow = i;
        for (auto k=i+1; k<n; k++) {
            if (abs(A[k][i]) > maxEl) {
                maxEl = abs(A[k][i]);
                maxRow = k;
            }
        }

        // Swap maximum row with current row (column by column)
        for (auto k=i; k<n+1;k++) {
            double tmp = A[maxRow][k];
            A[maxRow][k] = A[i][k];
            A[i][k] = tmp;
        }

        // Make all rows below this one 0 in current column
        for (auto k=i+1; k<n; k++) {
            double c = -A[k][i]/A[i][i];
            for (uint32_t j=i; j<n+1; j++) {
                if (i==j) {
                    A[k][j] = 0;
                } else {
                    A[k][j] += c * A[i][j];
                }
            }
        }
    }
std::cout<<"after m manipulation\n";
    // Solve equation Ax=b for an upper triangular matrix A
    std::vector<double> x(n,0);
    for (int32_t i=n-1; i>=0; i--) {
        std::cout<<"i: "<<i<<std::endl;
        x[i] = A[i][n]/A[i][i];
        std::cout<<"x[i]: "<<x[i]<<"\n";
        for (int32_t k=i-1;k>=0; k--) {
            std::cout<<"k: "<<k<<std::endl;
            A[k][n] -= A[k][i] * x[i];
            std::cout<<"A[k][n]: "<<A[k][n]<<"\n";
        }
    }
    std::cout<<"after equation solve\n";
    return x;
}

template<class Network>
void multiplex_decomposition(Network& net, std::vector<double> mux_angles, uint32_t target_id)
{
    uint32_t n = mux_angles.size();
    auto binarytogray = [](uint32_t num) { return (num>>1) ^ num;};
    //create M matrix
    std::vector<double> line(n+1,0);
    std::vector< std::vector<double> > M(n,line);
    for (auto i=0u; i<n; i++) {
        for (auto j=0u; j<n; j++) {
            M[i][j] = pow(-1, __builtin_popcount(i & (binarytogray(j)) ));
        }
    }
    //solving n equations n unknowns
    for (auto i=0u; i<n; i++) {
        M[i][n] = mux_angles[i];
    }
    std::cout<<"after M\n";
    std::vector<double> angs(n);
    angs = gauss(M);
    std::cout<<"after gauss\n";
    //add gates to network
    for(auto i=0u; i<n;i++){
        net.add_gate(gate_base(gate_set::rotation_y, angs[i]), target_id);
        uint32_t ctrl = log2(binarytogray(i) ^  ( (i==n-1) ? 0 : binarytogray(i+1) )) +1;
        ctrl += target_id;
        std::cout<<"control,target: "<<ctrl<<" "<<target_id<<std::endl;
        net.add_gate(gate::cx,ctrl,target_id);
    }
}

std::vector<uint32_t> extract_angle_idxs(std::vector<uint32_t> idxvec)
{
    std::vector<uint32_t> out_idx;
    auto temp=0;
    out_idx.emplace_back(temp);
    std::cout<<"size: "<<idxvec.size()<<std::endl;
    for(auto i=0u;i<idxvec.size();i++)
    {
        if(idxvec[i]==2)
        {
            for(auto j=0u;j<out_idx.size();j++)
            {
                auto temp1 = out_idx[j] + pow(2,i);
                out_idx.emplace_back(temp1);
            }
        }
        else
        {
            for(auto j=0u;j<out_idx.size();j++)
                out_idx[j] += (pow(2,i) * idxvec[i]);
        }
        
        
    }

    return out_idx;
}

template<typename Network>
void extract_multiplex_gates(Network & net, uint32_t n, std::vector < std::tuple < std::string,double,uint32_t,std::vector<uint32_t> > >
 gates)
{
    auto angle_1 = std::get<1> (gates[0]);
    auto targetid_1 = std::get<2> (gates[0]);
    auto csize_1 = (std::get<3> (gates[0])).size();
    if( (csize_1==0) && (targetid_1==(n-1)) )
    {
        if(angle_1 != 0)
            net.add_gate(gate_base(gate_set::rotation_y, angle_1), targetid_1);
        gates.erase(gates.begin());
    }

    for(int32_t i=n-2;i>=0;i--)
    {
        auto num_angles = pow (2, (n-i-1) );
        std::cout<<"num angles: "<<num_angles<<std::endl;
        std::vector<double> angles(num_angles,0);
        for( auto [name,angle,target_id,controls]: gates)
        { 
          if( target_id == uint32_t(i) )
            {
                auto len = n-target_id-1;
                std::vector<uint32_t> idxs (len,2);
                
                std::for_each(begin(controls), end(controls), [&idxs,n] (uint32_t c) {idxs[n-1-(c/2)] = c%2;});
                for(auto id : idxs)
                    std::cout<<"idxs: "<<id<<std::endl;
                std::cout<<"alaki\n";
                std::reverse(idxs.begin(),idxs.end());
                std::vector<uint32_t> angle_idxs = extract_angle_idxs(idxs);
                std::cout<<"alaki2\n";
                for(auto id : angle_idxs)
                    angles[id] = angle;

            }
        }
        for(auto id : angles)
            std::cout<<"angle: "<<id<<std::endl;
        multiplex_decomposition(net, angles, i);
    }
    
}

/*std::vector<std::tuple<std::string,double,uint32_t,std::vector<uint32_t>>>*/ void control_line_cancelling
(std::vector<std::tuple<std::string,double,uint32_t,std::vector<uint32_t>>>& in_gates, uint32_t nqubits)
{
    //std::vector<std::tuple<std::string,double,uint32_t,std::vector<uint32_t>>> out_gates;
    std::vector<uint32_t> line_values (nqubits, 0);
    uint32_t idx=0;
    for(auto & [name,angle,target_id,controls]: in_gates){
        if(angle==0){
            in_gates.erase(in_gates.begin()+idx);
            continue;
        }
        idx++;
        //check controls
        if (controls.size()>0){
            for(auto i=0u;i<controls.size();){
                auto l = controls[i] / 2;
                auto c_val = (controls[i] % 2); // 0:pos 1:neg
                if(line_values[l]==c_val){
                    std::cout<<"erase control\n";
                    controls.erase(controls.begin()+i);
                    continue;
                }
                i++;
            }
        }
        //update line values
        if( angle == M_PI )
            line_values[target_id] = 1;
        else 
            line_values[target_id] = 2;
    }

    return ;//out_gates;
}


} // namespace detail end
//**************************************************************
struct qsp_params {
	enum class strategy : uint32_t {
		allone_first,
		ownfunction,
	} strategy = strategy::ownfunction;
};

void general_qg_generation(std::map <uint32_t , std::vector < std::pair < double,std::vector<uint32_t> > > >& gates,
 kitty::dynamic_truth_table tt, uint32_t var_index, std::vector<uint32_t> controls 
 ,std::map<uint32_t , std::vector<std::pair<std::string, std::vector<int32_t>>>> dependencies)
{
    //-----co factors-------
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
                        auto index = dependencies[var_index][d].second[0];
                        gates[var_index].emplace_back(std::pair{ M_PI,std::vector{ dependencies[var_index][index].second[0]*2 +1} });
                    }
                    else if(dependencies[var_index][d].first == "not") // not cnot
                    {
                        auto index = dependencies[var_index][d].second[0];
                        gates[var_index].emplace_back(std::pair{ M_PI,std::vector{dependencies[var_index][index].second[0]*2 +1 } });
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{} });
                    }
                    else if(dependencies[var_index][d].first == "xor")
                    {
                        for( auto d_in=0u; d_in<dependencies[var_index][d].second.size(); d_in++)
                            gates[var_index].emplace_back(std::pair{ M_PI,std::vector{dependencies[var_index].second[d_in]*2 +1} }); 
                    }
                    
                    else if(dependencies[var_index].first == "xnor")
                    {
                        for( auto d=0u; d<dependencies[var_index].second.size(); d++)
                            gates[var_index].emplace_back(std::pair{ M_PI,std::vector{dependencies[var_index].second[d]*2 +1} });
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{} });
                    }
                    else if(dependencies[var_index].first == "and")
                    {
                        // to do --- insert nots
                        std::for_each(dependencies[var_index].second.begin(), dependencies[var_index].second.end(), [](uint32_t &el){el = abs(el)*2+1; });
                        gates[var_index].emplace_back(std::pair{ M_PI,dependencies[var_index].second });
                    }
                    else if(dependencies[var_index].first == "nand")
                    {
                        std::for_each(dependencies[var_index].second.begin(), dependencies[var_index].second.end(), [](uint32_t &el){el = el*2+1; });
                        gates[var_index].emplace_back(std::pair{ M_PI,dependencies[var_index].second });
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{} });
                    }
                    else if(dependencies[var_index].first == "or")
                    {
                        // to do --- insert nots
                        std::for_each(dependencies[var_index].second.begin(), dependencies[var_index].second.end(), [](uint32_t &el){el = abs(el)*2+1; });
                        gates[var_index].emplace_back(std::pair{ M_PI,dependencies[var_index].second });
                        for(auto d=0u; d<dependencies[var_index].second.size() ; d++)
                            gates[var_index].emplace_back(std::pair{ M_PI,std::vector{dependencies[var_index].second[d]} }); 
                    }
                    else if(dependencies[var_index].first == "nor")
                    {
                        std::for_each(dependencies[var_index].second.begin(), dependencies[var_index].second.end(), [](uint32_t &el){el = el*2+1; });
                        gates[var_index].emplace_back(std::pair{ M_PI,dependencies[var_index].second });
                        for(auto d=0u; d<dependencies[var_index].second.size() ; d++)
                            gates[var_index].emplace_back(std::pair{ M_PI,std::vector{dependencies[var_index].second[d]} }); 
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{} });
                    }
                    else if (dependencies[var_index].first == "and_xor")
                    {
                        // to do --- insert nots
                        std::for_each(dependencies[var_index].second.begin(), dependencies[var_index].second.end(), [](uint32_t &el){el = abs(el)*2+1; });
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector {dependencies[var_index].second[0],dependencies[var_index].second[1] } } );
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector{dependencies[var_index].second[2]} }); 
                    }
                    else if (dependencies[var_index].first == "and_xnor")
                    {
                        // to do --- insert nots
                        std::for_each(dependencies[var_index].second.begin(), dependencies[var_index].second.end(), [](uint32_t &el){el = abs(el)*2+1; });
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector {dependencies[var_index].second[0],dependencies[var_index].second[1] } } );
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector{dependencies[var_index].second[2]} }); 
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{} });
                    }
                    else if (dependencies[var_index].first == "or_xor")
                    {
                        // to do --- insert nots
                        std::for_each(dependencies[var_index].second.begin(), dependencies[var_index].second.end(), [](uint32_t &el){el = abs(el)*2+1; });
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector {dependencies[var_index].second[0],dependencies[var_index].second[1] } } );
                        gates[var_index].emplace_back(std::pair{ M_PI,std::vector{dependencies[var_index].second[0]} }); 
                        gates[var_index].emplace_back(std::pair{ M_PI,std::vector{dependencies[var_index].second[1]} }); 
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector{dependencies[var_index].second[2]} }); 
                        
                    }
                    else if (dependencies[var_index].first == "or_xnor")
                    {
                       // to do --- insert nots
                        std::for_each(dependencies[var_index].second.begin(), dependencies[var_index].second.end(), [](uint32_t &el){el = abs(el)*2+1; });
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector {dependencies[var_index].second[0],dependencies[var_index].second[1] } } );
                        gates[var_index].emplace_back(std::pair{ M_PI,std::vector{dependencies[var_index].second[0]} }); 
                        gates[var_index].emplace_back(std::pair{ M_PI,std::vector{dependencies[var_index].second[1]} }); 
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector{dependencies[var_index].second[2]} }); 
                        gates[var_index].emplace_back(std::pair{ M_PI, std::vector<uint32_t>{} });
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
    auto ctrl0 = var_index*2 + 0; //negetive control: /2 ---> index %2 ---> sign
    controls_new0.emplace_back(ctrl0);
    if (c0_allone){
        
        //---add H gates---
        for(auto i=0u;i<var_index;i++)
            //gates.emplace_back("RY",M_PI/2,i,controls_new0);
            gates[i].emplace_back(std::pair{M_PI/2,controls_new0});
        //--check one cofactor----
        std::vector<uint32_t> controls_new1;
        std::copy(controls.begin(), controls.end(), back_inserter(controls_new1)); 
        auto ctrl1 = var_index*2 + 1; //positive control: /2 ---> index %2 ---> sign
        controls_new1.emplace_back(ctrl1);
        if(c1_allone){
            //---add H gates---
            for(auto i=0u;i<var_index;i++)
                //gates.emplace_back("RY",M_PI/2,i,controls_new1);
                gates[i].emplace_back(std::pair{M_PI/2,controls_new1});

        }
        else if(c1_allzero){
            return;
        }
        else{//some 1 some 0
            general_qg_generation(gates,tt1,var_index-1,controls_new1 , dependencies);
        }
    }
    else if(c0_allzero){
        //--check one cofactor----
        std::vector<uint32_t> controls_new1;
        std::copy(controls.begin(), controls.end(), back_inserter(controls_new1)); 
        auto ctrl1 = var_index*2 + 1; //positive control: /2 ---> index %2 ---> sign
        controls_new1.emplace_back(ctrl1);
        if(c1_allone){
            //---add H gates---
            for(auto i=0u;i<var_index;i++)
                //gates.emplace_back("RY",M_PI/2,i,controls_new1);
                gates[i].emplace_back(std::pair{M_PI/2,controls_new1});

        }
        else if(c1_allzero){
            return;
        }
        else{//some 1 some 0
            general_qg_generation(gates,tt1,var_index-1,controls_new1 , dependencies);
        }
    }
    else{//some 0 some 1 for c0
        
        std::vector<uint32_t> controls_new1;
        std::copy(controls.begin(), controls.end(), back_inserter(controls_new1)); 
        auto ctrl1 = var_index*2 + 1; //positive control: /2 ---> index %2 ---> sign
        controls_new1.emplace_back(ctrl1);
        if(c1_allone){
            general_qg_generation(gates,tt0,var_index-1,controls_new0 , dependencies);
            //---add H gates---
            for(auto i=0u;i<var_index;i++)
                //gates.emplace_back("RY",M_PI/2,i,controls_new1);
                gates[i].emplace_back(std::pair{M_PI/2,controls_new1});

        }
        else if(c1_allzero){
            general_qg_generation(gates,tt0,var_index-1,controls_new0 , dependencies);
            //return;
        }
        else{//some 1 some 0
            general_qg_generation(gates,tt0,var_index-1,controls_new0 , dependencies);
            general_qg_generation(gates,tt1,var_index-1,controls_new1 , dependencies);
        }
    }

}

template<typename Network>
void qc_generation(Network & net, std::vector < std::tuple < std::string,double,uint32_t,std::vector<uint32_t> > >
 gates){
    for(const auto [name,angle,target_id,controls]: gates){
        if(name=="RY"){
            
            if(controls.size()==0){
                auto temp = gate_base(gate_set::rotation_y, angle);
                net.add_gate(temp, target_id);
            }
            else if( (controls.size()==1) && (angle==M_PI) ){
                
                    if(controls[0]%2 == 0){
                        net.add_gate(gate::pauli_x, controls[0]/2);
                        net.add_gate(gate::cx,controls[0]/2,target_id);
                        net.add_gate(gate::pauli_x, controls[0]/2);
                    }
                    else
                    {
                        net.add_gate(gate::cx,controls[0]/2,target_id);
                    }
                           
            }
            else {//we have multi control probability gate
                unsigned nlines = controls.size()+1;
                auto tt = kitty::create<kitty::dynamic_truth_table>(nlines-1);
                uint32_t tt_idx_set=0;
                //kitty::set_bit(tt,pow(2,nlines-1)-1);
            
                std::vector<qubit_id> q_map;
                net.add_gate(gate_base(gate_set::rotation_y, angle/2), target_id);
                auto idx=0;
                for(const auto ctrl:controls){
                    if(ctrl%2 == 1){//negative control
                        //net.add_gate(gate::pauli_x, ctrl/2)
                        tt_idx_set += pow(2,idx);
                    }
                    idx++;
                    q_map.emplace_back(ctrl/2);
                }
                q_map.emplace_back(target_id);
                tt_idx_set += pow(2,nlines-1);
                kitty::set_bit(tt,tt_idx_set);
                detail::decomposition_mcz(net,q_map,tt);
                //for(const auto ctrl:controls){
                    //if(ctrl%2 == 1)//negative control
                        //net.add_gate(gate::pauli_x, ctrl/2);
                //}
                net.add_gate(gate_base(gate_set::rotation_y, -angle/2), target_id);

            }//end multi control
        }//end RY
        else if(name=="H"){
            
            if(controls.size()==0)
                net.add_gate(gate::hadamard, target_id);
            else{//we have multi control probability gate
                unsigned nlines = controls.size()+1;
                auto tt = kitty::create<kitty::dynamic_truth_table>(nlines-1);
                uint32_t tt_idx_set=0;
                std::vector<qubit_id> q_map;
                net.add_gate(gate_base(gate_set::rotation_y, M_PI/4), target_id);
                auto idx=0;
                for(const auto ctrl:controls){
                    if(ctrl%2 == 1){//negative control
                        //net.add_gate(gate::pauli_x, ctrl/2)
                        tt_idx_set += pow(2,idx);
                    }
                    idx++;
                    q_map.emplace_back(ctrl/2);
                }
                q_map.emplace_back(target_id);
                tt_idx_set += pow(2,nlines-1);
                kitty::set_bit(tt,tt_idx_set);
                detail::decomposition_mcz(net,q_map,tt);
                // for(const auto ctrl:controls){
                //     if(ctrl%2 == 1)//negative control
                //         net.add_gate(gate::pauli_x, ctrl/2);
                // }
                net.add_gate(gate_base(gate_set::rotation_y, -M_PI/4), target_id);
            }//end multi control
        }//end H

    }// end gates

}

template<typename Network>
void qsp_ownfunction( Network& net, 
const std::string &tt_str, std::map<uint32_t , std::vector<std::pair<std::string, std::vector<int32_t>>>> const dependencies, 
qsp_tt_statistics& stats)
{
    std::map <uint32_t , std::vector < std::pair < double,std::vector<uint32_t> > > > gates; // gate name, angle, target_id, controls:id and sign /2 and %2
    auto tt_vars = int(log2(tt_str.size()));
    
    kitty::dynamic_truth_table tt(tt_vars);
    kitty::create_from_binary_string( tt, tt_str);

    auto var_idx = tt_vars-1;
 
    std::vector<uint32_t> cs;

    //std::cout<<"debug: before qg generation\n";
    general_qg_generation(gates,tt,var_idx,cs , dependencies);
    //std::cout<<"debug: after qg generation\n";
    //detail::control_line_cancelling(gates,tt_vars);
    auto total_rys = 0;
    auto total_cnots = 0;

    bool sig;
    auto n_reduc = 0;

    std::vector< std::pair<uint32_t,uint32_t> > gates_num(tt_vars); //rys,cnots
    for(auto i=0u;i<gates.size();i++)
    {
        //std::cout<<"gates: "<<i<<"\n";
        //std::cout<<"num: "<<gates[i].size()<<std::endl;
        if(gates[i].size()==0)
        {
            gates_num[i] = std::make_pair(0,0);
            n_reduc++;
            continue;
        }

        auto rys = 0;
        auto cnots = 0;
        sig = 1;
        
        for(auto j=0u; j< gates[i].size(); j++)
        {
            if(gates[i][j].second.size()==(gates.size()-1-i-n_reduc))
            {
                sig = 0;
            }
            //std::cout<<"angle:  "<<gates[i][j].first <<std::endl<<"ctrls: ";
            //for(auto k=0; k<gates[i][j].second.size() ; k++)
                //std::cout<<gates[i][j].second[k]<<"  ";
                
                //std::cout<<gates[i][j].second.size()<<"  ";
            //std::cout<<std::endl;
            auto cs = gates[i][j].second.size();
            if(cs==0)
                rys += 1;
            else if (cs==1 && gates[i][j].first == M_PI)
                cnots += 1;
            else
            {
                rys += pow(2,cs);
                cnots += pow(2,cs);
            }
            
        }
        if (sig==0)
        {
            if(i==gates.size()-1-n_reduc)
            {
                cnots = 0;
                rys = 1;
            }
            else if(i==gates.size()-2-n_reduc)
            {
                if(gates[i].size()==1 && gates[i][0].first==M_PI)
                    cnots = 1;
                else
                {
                    rys = pow(2,(gates.size()-1-i-n_reduc));
                    cnots = pow(2,(gates.size()-1-i-n_reduc)); 
                }
            }
            else
            {
                rys = pow(2,(gates.size()-1-i-n_reduc));
                cnots = pow(2,(gates.size()-1-i-n_reduc)); 
            }
            
        }  
        
        gates_num[i] = std::make_pair(rys,cnots);
            //out_file<<"i:"<<i<<"  "<<rys<<"  "<<cnots<<"  ";

        total_rys += rys;
        total_cnots += cnots;  
    }
    
    // std::cout<<"n_reduc: "<<n_reduc<<std::endl;
    stats.total_cnots += total_cnots;
    stats.total_rys += total_rys;

    stats.reduction += n_reduc;
    
    if(total_cnots < (pow(2,gates.size()-n_reduc)-2) ) 
    {
        for(auto i=0u; i<gates_num.size() ; i++)
        {
          // out_file<<"i:"<<i<<"  "<<gates_num[i].first<<"  "<<gates_num[i].second<<"  ";  
        }
    
        // out_file<<"trys: "<<total_rys<<"  upper: "<<pow(2,gates.size()-n_reduc)-1<<
        // "  tcnots: "<<total_cnots<<"  upper: "<<pow(2,gates.size()-n_reduc)-2;
        // out_file<<std::endl;

        funcdep_bench_useful++;
    }
   
    else
    {
        if(dependencies.size()>0)
        {
           funcdep_bench_notuseful++;
        }
    }
    all_bench++;

    // std::cout<<"all benches: "<<all_bench<<std::endl;
    // std::cout<<"funcdep useful benches: "<<funcdep_bench_useful<<std::endl;
    // std::cout<<"funcdep not useful benches: "<<funcdep_bench_notuseful<<std::endl;
    
    //detail::extract_multiplex_gates(net,tt_vars,gates);  
}

/*template<typename Network>
void qsp_allone_first(Network & net, const std::string &tt_str , 
std::vector<std::pair<std::string, std::vector<int>>> dependencies)
{
    std::vector < std::tuple < std::string,double,uint32_t,std::vector<uint32_t> > > gates; // gate name, angle, target_id, controls:id and sign /2 and %2
    auto tt_vars = int(log2(tt_str.size()));
    
    //first tt
    kitty::dynamic_truth_table tt(tt_vars);
    kitty::create_from_binary_string(tt, tt_str);
    auto ones = kitty::count_ones(tt);

    auto tt_new = kitty::create<kitty::dynamic_truth_table>(tt_vars);
    for(auto i=0u;i<ones;i++)
        kitty::set_bit(tt_new,i);

    auto var_idx = tt_vars-1;
    std::vector<uint32_t> cs;
    general_qg_generation(gates,tt_new,var_idx,cs);
    //detail::control_line_cancelling(gates,tt_vars);
    for(auto i=0u;i<gates.size();i++){
        std::cout<<"gates: "<<i<<"\n";
        std::cout<<std::get<0> (gates[i]) <<std::endl;
        std::cout<<"angle:  "<<std::get<1> (gates[i]) <<std::endl;
        std::cout<<"target_id:  "<<std::get<2> (gates[i]) <<std::endl;
        for(auto j=0;j< (std::get<3> (gates[i])).size();j++)
            std::cout<<(std::get<3> (gates[i])) [j] <<"  ";
            std::cout<<std::endl;
    }
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
                          
}*/


template<class Network>
void qsp_tt_dependencies(Network& network, const std::string &tt_str , 
std::map<uint32_t , std::vector<std::pair<std::string, std::vector<int32_t>>>> const dependencies, qsp_tt_statistics& stats)
{
    //assert(tt_str.size() <= pow(2,6));
    const uint32_t num_qubits = std::log2(tt_str.size());
    for (auto i = 0u; i < num_qubits; ++i) 
    {
      network.add_qubit();
    }
	
    // switch (params.strategy) 
    // {
    // 	case qsp_params::strategy::allone_first:
    //         std::cout<<"one first\n";
    // 		qsp_allone_first(network, tt_str , dependencies);
    // 		break;
    // 	case qsp_params::strategy::ownfunction:
    //         std::cout<<"own func\n";

    stopwatch<>::duration time_bdd_traversal{0};
    {
      stopwatch t( time_bdd_traversal );
      qsp_ownfunction( network, tt_str, dependencies, stats );
    }

    stats.time = to_seconds( time_bdd_traversal );
}

} // namespace tweedledum end
