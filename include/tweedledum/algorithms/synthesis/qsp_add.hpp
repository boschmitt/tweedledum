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
#include <QMDDpack_fereshte.h>
#include <tweedledum/utils/stopwatch.hpp>
#include <typeinfo>

namespace tweedledum {
namespace detail {

BDD create_bdd_from_pla(Cudd &cudd , std::string file_name , uint32_t & num_inputs)
{
    std::ifstream infile(file_name);
    std::string in,out;
    infile>>in>>out;
    num_inputs = std::atoi(out.c_str());
    int num_outputs = 1;
    //std::cout<<"num "<<num_inputs<<std::endl;

    //-------
    //Cudd cudd;
    auto mgr = cudd.getManager();
    BDD output;
    auto bddNodes = new BDD[num_inputs];
    for(int i=num_inputs-1; i >= 0 ; i--)
    {
        bddNodes[i] = cudd.bddVar(); // index 0: LSB
    }
    //-------

    bool sig = 1;
    while(infile>>in>>out)
    {
        //std::cout<<"in: "<<in<<"  out: "<<out<<std::endl;

        BDD tmp,var;
        
        bool sig2=1;

        for ( auto i = 0u; i < num_inputs; ++i )
        {
          if ( in[i] == '-' ) continue;
          var = bddNodes[i];

          if ( in[i] == '0' ) var =  !var; 
          if(sig2)
          {
              tmp = var; 
              sig2=0;
          }
          
          else
            tmp &= var;
          
        }

        if(sig)
        {
            output = tmp;
            sig=0;
            
        }
        else
            output |= tmp ;

        
    }
    return output;
}

BDD create_bdd_from_tt(Cudd &mgr , std::string tt_str , uint32_t & numvar )
{
    numvar = std::log2(tt_str.size());
    auto bddNodes = new BDD[numvar];
    for(int i=numvar-1; i >= 0 ; i--)
    {
        bddNodes[i] = mgr.bddVar(); // index 0: LSB
    }

    BDD f_bdd;
    int sig=1;;
    for(auto i=0 ; i<tt_str.size() ; i++)
    {
        if(tt_str[i] == '1')
        {
            auto n = i;
            BDD temp;
            temp = ((n&1) == 1) ? bddNodes[0] : !bddNodes[0];
            n >>= 1;
            for(auto j=1 ; j<numvar ; j++)
            {
                temp &= ((n&1) == 1) ? bddNodes[j] : !bddNodes[j];
                n >>= 1;
            }

            if(sig)
            {
                f_bdd = temp;
                sig = 0;
            }
            else
            {
                f_bdd |= temp;
            }
            
        }
    }

    return f_bdd;
} // end function create bdd

void count_ones_add_nodes(std::unordered_set<DdNode*>& visited, 
std::vector<std::map<DdNode *,uint32_t>>  &node_ones, 
DdNode *f, uint32_t num_vars , std::vector<uint32_t> orders)
{
    
    auto current = f;

    if ( visited.count( current ) ) 
        return;
    if ( Cudd_IsConstant( current ) ) 
        return;
    
    //std::cout<<"current index: "<<current->index<<std::endl;
    count_ones_add_nodes( visited, node_ones, cuddE( current ), num_vars , orders);
    count_ones_add_nodes( visited, node_ones , cuddT( current ), num_vars , orders);
    
    visited.insert( current );
    uint32_t Eones = 0;
    uint32_t Tones = 0;

    if( Cudd_IsConstant(cuddT(current)))
    {
        if(Cudd_V(cuddT(current)))
        {
            auto temp_pow = num_vars - orders[current->index] - 1;
            Tones = pow(2,temp_pow) * 1 ;  
        }
    }
    else{
        auto temp_pow = orders[cuddT(current)->index] - 1 - orders[current->index];
        auto Tvalue = 0;
        Tvalue = node_ones[cuddT(current)->index].find(cuddT(current))->second;
        Tones = pow(2,temp_pow) * Tvalue;
    }
    
    if( Cudd_IsConstant(cuddE(current)))
    {
        if (Cudd_V(cuddE(current)))
        {
            auto temp_pow = num_vars - orders[current->index] - 1; 
            Eones = pow(2,temp_pow) * 1 ;
        }
        
    }
    else{
        auto temp_pow = orders[cuddE(current)->index] - 1 - orders[current->index];
        auto max_ones = pow(2,num_vars- orders[cuddE(current)->index]);
        auto Evalue = 0;
        Evalue = node_ones[cuddE(current)->index].find(cuddE(current))->second;
        Eones = pow(2,temp_pow) * Evalue;
    }
    //std::cout<<"index: "<<current->index<<"  ";
    //std::cout<<"weight: "<< Tones+Eones <<std::endl;
    node_ones[current->index].insert({current,Tones+Eones});
} /* end of count_ones_add_nodes function */

void extract_probabilities_and_MCgates(std::unordered_set<DdNode*>& visited,
std::vector<std::map<DdNode *,uint32_t >> node_ones,
std::map< DdNode* , std::vector < std::vector< std::pair <double , std::vector<int32_t> >> > > &gates,
DdNode *f, uint32_t num_vars , std::vector<uint32_t> orders)
{
    auto current = f;
    if ( visited.count(current) ) 
        return;
    if ( Cudd_IsConstant(current) ) 
        return;

    extract_probabilities_and_MCgates( visited , node_ones , gates , cuddE(current) , num_vars , orders );
    extract_probabilities_and_MCgates( visited , node_ones , gates , cuddT(current) , num_vars , orders );

    visited.insert(current);
    double ep = 0;
    double dp = node_ones[current->index].find(current)->second;
    if( Cudd_IsConstant(cuddT(current)) && Cudd_V(cuddT(current)) ) //?? -> one probability
    {
        ep = pow(2 , num_vars-1-orders[current->index]);
    }
    else if( Cudd_IsConstant(cuddT(current)) && !Cudd_V(cuddT(current)) ) //?? -> one probability
    {
        ep = 0;
    }
    else
    {
        ep = node_ones[cuddT(current)->index].find(cuddT(current))->second * 
        (pow(2 , orders[cuddT(current)->index] - orders[current->index] - 1));  //?? -> one probability
    }
    
    double p = ep/dp;

    //gates_pre[current->index].insert({current, std::make_tuple( 1-p, cuddE(current) , cuddT(current) ) });

    auto const it = gates.find( current );
    if ( it == gates.end() )
    {
        
        gates.emplace( current, std::vector<std::vector<std::pair<double, std::vector<int32_t>>>>( num_vars ) );
    }

    /* inserting current single-qubit G(p) gate */
    if(p != 0)
    {
        gates[current][current->index].emplace_back( std::make_pair<double, std::vector<int32_t>>(1-p , {}) );
    }
    /* inserting childs gates */
    if( !Cudd_IsConstant(cuddE(current)) )
    {
        for(auto i=0 ; i<num_vars ; i++)
        {
            if(gates[cuddE(current)][i].size() != 0)
            {
                for(auto j=0 ; j<gates[cuddE(current)][i].size() ; j++)
                {
                    auto pro = gates[cuddE(current)][i][j].first;
                    std::vector<int32_t> controls;
                    for(auto k=0 ; k<gates[cuddE(current)][i][j].second.size() ; k++)
                        controls.emplace_back(gates[cuddE(current)][i][j].second[k]);
                    //std::copy(gates[cuddE(current)][i][j].second.begin() , gates[cuddE(current)][i][j].second.end() , controls);
                    //auto controls = gates[cuddE(current)][i][j].second;
                    controls.emplace_back(-(current->index+1));
                    gates[current][i].emplace_back( std::make_pair(pro,controls) );

                }
            
            }
        }
    }

    if( !Cudd_IsConstant(cuddT(current)) )
    {
        for(auto i=0 ; i<num_vars ; i++)
        {
            if(gates[cuddT(current)][i].size() != 0)
            {
                for(auto j=0 ; j<gates[cuddT(current)][i].size() ; j++)
                {
                    auto pro = gates[cuddT(current)][i][j].first;
                    std::vector<int32_t> controls;
                    for(auto k=0 ; k<gates[cuddT(current)][i][j].second.size() ; k++)
                        controls.emplace_back(gates[cuddT(current)][i][j].second[k]);
                    //std::copy(gates[cuddT(current)][i][j].second.begin() , gates[cuddT(current)][i][j].second.end() , controls);
                   
                    //auto controls = gates[cuddT(current)][i][j].second;
                    controls.emplace_back(current->index+1);
                    gates[current][i].emplace_back( std::make_pair(pro,controls) );

                }
            
            }
        }
    }

    /* inserting Hadamard gates */
    auto Edown = Cudd_IsConstant(cuddE(current)) ? num_vars : orders[cuddE(current)->index];
    auto Tdown = Cudd_IsConstant(cuddT(current)) ? num_vars : orders[cuddT(current)->index];
    uint32_t E_num_value = 1;
    uint32_t T_num_value = 1;
    if(Cudd_IsConstant(cuddE(current)) && !Cudd_V(cuddE(current)))
        E_num_value = 0;
    if(Cudd_IsConstant(cuddT(current)) && !Cudd_V(cuddT(current)))
        T_num_value = 0;
    for(auto i = orders[current->index]+1 ; i < Edown ; i++)
    {
        auto id = std::find(orders.begin() , orders.end() , i) - orders.begin();
        if (E_num_value != 0)
        {
            //gates_pre[id].insert({current, std::make_tuple( 1/2.0, nullptr , nullptr ) });
            std::vector<int32_t> temp_c;
            temp_c.emplace_back(-(current->index+1) );
            gates[current][id].emplace_back( std::make_pair(1/2.0 , temp_c ) );
            
        }
            
    }
    for(auto i = orders[current->index]+1 ; i < Tdown ; i++)
    {
        auto id = std::find(orders.begin() , orders.end() , i) - orders.begin();
        if (T_num_value != 0)
        {
            //gates_pre[id].insert({current, std::make_tuple( 1/2.0, nullptr , nullptr ) });
            std::vector<int32_t> temp_c;
            temp_c.emplace_back(current->index+1);
            gates[current][id].emplace_back( std::make_pair(1/2.0 , temp_c ) );
            
        }
    }

    
    
} /* end of extract_gates function */

std::vector<double> make_angles_vector(std::vector<std::pair<double , std::vector<int32_t>>> gates , uint32_t n)
{
    std::cout<<"angles1: ";
    for(auto k=0 ; k<gates.size() ; k++)
    {
        std::cout<<gates[k].first<<" ";
        for(auto z=0; z<gates[k].second.size() ; z++)
            std::cout<<gates[k].second[z]<<" ";
         std::cout<<std::endl;
    }
    std::cout<<std::endl;

    std::vector<double> angs (pow(2,n) , 0);
    for(auto i=0u ; i<gates.size() ; i++) //??
    {
        std::vector<int> bin(n,-1);
        for(auto j=0u ; j<gates[i].second.size() ; j++)
        {
            
            if(gates[i].second[j]>0)
                bin[n - gates[i].second[j]] = 1;
            else
            {
                bin[n + gates[i].second[j]] = 0;
            }  
        }
        std::vector<int> dontcares;
        int index = 0;
        for(auto k=0u ; k<n ; k++)
        {
            if(bin[k]==-1)
                dontcares.emplace_back(pow(2,k));
            else
            {
                index += bin[k] * pow(2,k);
            }      
        }
        std::vector<int> idxs;
        if(dontcares.size()==0)
            idxs.emplace_back(index);
        else
        {
            for(auto l=0u ; l<pow(2,dontcares.size()) ; l++)
            {
                auto res = index;
                int dcid=0;
                auto temp = l;
                while(temp>0)
                {
                    res += (temp & 1) * dontcares[dcid];
                    dcid++;
                    temp = temp >> 1;
                }
                idxs.emplace_back(res);
            }
        }
        for(auto m=0 ; m<idxs.size() ; m++)
        {
            angs[idxs[m]] = 2*acos(sqrt(gates[i].first)); //??
            
        }
        //std::cout<<std::endl;
    }

    std::cout<<"angles2: ";
    for(auto k=0 ; k<angs.size() ; k++)
    {
        std::cout<<angs[k]<<" ";
    }
    std::cout<<std::endl;
    return angs;
}


template<class Network>
void multiplex_decomposition (Network &net , 
std::vector< std::vector<std::pair<double , std::vector<int32_t>>> > gates)
{
    QMDD_init(gates.size());
std::cout<<"vars: "<<gates.size()<<std::endl;
    /* add first rotation gate */
    double ang = 2*acos(sqrt(gates[0][0].first)); //??
    std::cout<<"first ang: "<<ang<<"//"<<(ang/M_PI)*180<<std::endl;
    net.add_gate(gate_base(gate_set::rotation_y, ang), qubit_id(0));
    
    qmdd res;
    res.index=0;
    int var;
std::cout<<"before H"<<std::endl;
    edge hq = QMDDmake_HadamardGate(res);
std::cout<<"after H: "<<hq.idx_next<<std::endl;
    // Generate Gray code
	std::vector<uint32_t> gray_code;
	for (auto i = 0u; i < (1u << gates.size()); ++i) 
    {
		gray_code.emplace_back((i >> 1) ^ i);
	}

    for(auto i=1u ; i<gates.size() ; i++)
    {
        if(gates[i][0].second.size()==0)
        {
            double ang = 2*acos(sqrt(gates[i][0].first)); //??
            std::cout<<"ang "<<i<<": "<<ang<<std::endl;
            net.add_gate(gate_base(gate_set::rotation_y, ang ), qubit_id(i));
        }
        else
        {
            var = i;

            for(auto j=0; j<gates[i].size();j++)
            {
                std::cout<<gates[i][j].first<<": ";
                for(auto k=0 ; k<gates[i][j].second.size() ; k++)
                {
                    std::cout<<gates[i][j].second[k]<<"  ";
                }
                std::cout<<std::endl;
            }
            //auto col_vector = make_angles_vector(gates[i],i);

            
            
            //edge cq = QMDDmake_ColumnVector (res, col_vector , 0 , pow(2,var)-1 , var );
            std::cout<<"nodes: "<<gates[i].size()<<std::endl;
            uint32_t nodes=0;
            edge cq = QMDDmake_reducedColumnVector(res , gates[i] , i , i , nodes);
            std::cout<<"qmdd nodes: "<<nodes<<std::endl;
            std::vector<double> angles1;
            extract_column(res , cq , angles1);
            for(auto i=0u ; i<angles1.size() ; i++)
            std::cout<<angles1[i]<<"  ";
            std::cout<<"\n";
            
            if(i>1)
                hq = QMDDmake_Hadamard_AddNode(res , hq , i-1);
            
            edge mul = QMDDmultiply(res , hq , cq , var);
            std::vector<double> angles;
            extract_column(res , mul , angles);
            
            //------------
            std::cout<<"real angles ["<<i<<"]";
            std::cout<<"angles: "<<angles.size()<<std::endl;
            for(auto j=0u ; j<angles.size() ; j++)
            {
                uint32_t location;
                if(j== (angles.size() - 1))
                    location = std::log2(gray_code[j] ^ gray_code[0]);
                else
                {
                    location = std::log2(gray_code[j] ^ gray_code[j+1]);
                }
                net.add_gate(gate_base(gate_set::rotation_y, angles[gray_code[j]] * (pow(2,-var)) ) , qubit_id(i));
                net.add_gate(gate::cx , qubit_id(i-location-1) , qubit_id(i) );   
                auto temp = angles[gray_code[j]] * (pow(2,-var));
                std::cout<< temp <<"//"<<(temp/M_PI)*180<<"  ";
            }
            std::cout<<std::endl;
            //------------
        }
    }
    
  
} /* end of multiplex_decomposition function */

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
            //std::cout<<"angle: "<<gates[i][j].first<<std::endl;
        }
    }
    
}


template<class Network>
void qsp_add(Network& network , const std::string tt_str) //, const std::string &tt_str, qsp_params params = {})
{
   
   /*
    Network initialization
    */
    //assert(tt_str.size() <= pow(2,6));
    uint32_t num_qubits ; 
    
	/*
    Create BDD
    */
   //---create bdd from tt string
    Cudd cudd;
    auto mgr = cudd.getManager();
    //std::cout<<"qubits: "<<num_qubits<<std::endl;
    auto f_bdd = detail::create_bdd_from_tt(cudd , tt_str , num_qubits);
    // auto d = mgr.bddVar(); //MSB
    // auto c = mgr.bddVar();
    // auto b = mgr.bddVar();
    // auto a = mgr.bddVar(); //LSB

    
    //std::cout<<"BDD created\n";
    
    
    //Cudd_ReduceHeap(mgr,CUDD_REORDER_EXACT,0);
    //Cudd_PrintGroupedOrder(mgr, "BDD" , NULL);

    

   //----cretae bdd from pla file---
   std::string file_name = "ghz15.txt";
   //auto f_bdd = detail::create_bdd_from_pla(cudd , file_name , num_qubits);
    
   
    std::vector<uint32_t> orders;
    for(auto i=0u ; i<cudd.ReadSize() ; i++)
        orders.emplace_back(Cudd_ReadPerm(mgr , i));

        
    std::cout<<"qubits: "<<num_qubits<<std::endl;
    auto f_add = Cudd_BddToAdd(mgr,f_bdd.getNode());

    //std::cout<<"ADD created\n";
    
    FILE *outfile; // output file pointer for .dot file
    outfile = fopen("graph.dot","w");
    DdNode **ddnodearray = (DdNode**)malloc(sizeof(DdNode*)); // initialize the function array
    ddnodearray[0] = f_add;
    Cudd_DumpDot(mgr, 1, ddnodearray, NULL, NULL, outfile); // dump the function to .dot file

    //------create network------
    for (auto i = 0u; i < num_qubits; ++i) {
		network.add_qubit();
	}
    //--------------------------
    
    std::unordered_set<DdNode*> visited1;
    //std::vector< std::vector<std::pair<double , std::vector<int32_t>>> > gates (num_qubits);
    std::vector< std::map < DdNode* , std::tuple<double , DdNode* , DdNode*> > > gates_pre(num_qubits); 
    std::map< DdNode* , std::vector < std::vector< std::pair <double , std::vector<int32_t> >> > > gates;
    std::vector<int32_t> controls;

    stopwatch<>::duration time_bdd_traversal{0};
    {
        stopwatch t( time_bdd_traversal );
        std::vector<std::map <DdNode * , uint32_t > > node_ones(num_qubits);
        std::vector<std::pair<uint32_t,uint32_t>> ones_frac;
        std::unordered_set<DdNode*> visited;
        detail::count_ones_add_nodes( visited , node_ones , f_add , num_qubits , orders);

        //std::cout<<"ones counted\n";
        

        detail::extract_probabilities_and_MCgates(visited1 , node_ones , gates ,/* controls , */ f_add , num_qubits , orders);
        
    }
    
    std::cout << "time for BDD traversal = " << to_seconds( time_bdd_traversal ) << "s" << std::endl;
    //detail::multiplex_decomposition(network,gates);
    std::cout<<"nodes: "<<Cudd_DagSize(f_add)<<std::endl;
        
    auto total_gates=0;
    auto Rxs = 0;
    auto Rys = 0;
    auto Ts = 0;
    auto CNOTs = 0;
    auto Ancillaes = 0;

    for(auto i=1u ; i<cudd.ReadSize() ; i++)
    {
        total_gates += gates[f_add][i].size();

        //std::cout<<"line: "<<i<<"  gates: "<<gates[f_add][i].size()<<"\n";
        if(gates[f_add][i].size() < (pow(2,i)/(6*(i+1)-12)) )
        {
            for(auto j=0u ; j<gates[f_add][i].size() ; j++)
            {
                //std::cout<<gates[f_add][i][j].first<<": ";
                //for(auto k=0u ; k<gates[f_add][i][j].second.size() ; k++)
                    //std::cout<<gates[f_add][i][j].second[k]<<"  ";
                //std::cout<<"\n";
                auto cs = gates[f_add][i][j].second.size();

                if(cs==1)
                {
                    CNOTs += 1;
                }
                else if(cs==2)
                {
                    CNOTs += 6;
                    Ts += 7;
                }
                else if(cs==3)
                {
                    CNOTs += 12;
                    Ts += 15;
                    Ancillaes += 1;
                }
                else
                {
                    cs += 1;
                    CNOTs += (6*cs-12);
                    Ts += (8*cs-17);
                    Ancillaes += (floor((cs-3)/2));
                }  
            }
            Rxs += gates[f_add][i].size()+1;
        }
        else
        {
                CNOTs += pow(2,i);
                Rys += pow(2,i);
        }
    }

    std::cout<<"total MC gates: "<<total_gates<<std::endl;
    std::cout<<"total CNOT: "<<CNOTs<<std::endl;
    std::cout<<"total sqg: "<<Rys+Rxs+Ts<<std::endl;
    std::cout<<"total Ry: "<<Rys<<std::endl;
    std::cout<<"total Rx: "<<Rxs<<std::endl;
    std::cout<<"total T: "<<Ts<<std::endl;
    std::cout<<"total ancillae: "<<ceil((num_qubits-3)/2.0)<<std::endl;

}

} /* namespace tweedledum end */
