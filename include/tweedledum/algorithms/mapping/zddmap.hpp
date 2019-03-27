/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken, Kate Smith
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/mcst_gate.hpp"
#include "../../io/quil.hpp"
#include "../../io/write_unicode.hpp"
#include "../../networks/netlist.hpp"
#include "../../utils/dd/zdd.hpp"
#include "../../utils/device.hpp"
#include "../../views/pathsum_view.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace tweedledum {

struct find_maximal_partitions_params
{
        std::string mapped_filename = "zdd_mapped.quil";

        bool verbose = false;
        bool very_verbose = false;
};

struct find_maximal_partitions_stats
{
        double time_swap_layers_built = 0.0;
        double time_map_search = 0.0;
        double time_new_crk = 0;
        double time_total = 0.0;
};

namespace detail
{

template<typename Ntk>
class find_maximal_partitions_impl {
public:
	find_maximal_partitions_impl(Ntk const& circ, device const& arch, find_maximal_partitions_params const& ps, find_maximal_partitions_stats& st)
	    : circ_(circ)
	    , arch_(arch)
            , ps_(ps)
            , st_(st)
	    , zdd_(circ.num_qubits() * arch.num_vertices, 21)
	    , from_(circ.num_qubits())
	    , to_(arch.num_vertices)
            , edge_perm_(arch.num_vertices, 0)
	    , fmt_(circ.num_qubits())
	{
		std::iota(edge_perm_.begin(), edge_perm_.end(), 0);
	}

	auto run(){
		auto start = std::chrono::system_clock::now(); //start of function
                
                uint32_t depth_weight = 0;
                uint32_t map_weight = 1;
		double swap_weight = 1;

                // zdd_.build_tautologies();
		init_from();
		init_to();
		init_valid();
		zdd_.garbage_collect();
		init_bad();
		//for (auto& t : to_)
		//	zdd_.deref(t);
		zdd_.garbage_collect();

		std::vector<zdd_base::node> mappings;
		uint32_t c, t;
		auto m = zdd_.bot();

		//vectors that hold sets found in mapping and swap layer zdds,respectively
		std::vector< std::vector<uint32_t>> global_found_sets;
		std::vector< std::vector<uint32_t>> global_swap_layers;
		
        	//counts the gates
        	uint32_t ctr = 0;
        	//vector that holds gate number that swap is needed for
        	std::vector<uint32_t> index_of_swap;
        	//vector that holds the qubits implemented swaps
        	std::vector<std::vector<uint32_t>> swapped_qubits;
                //vector that holds index of new maps
                std::vector<uint32_t> index_new_map;
                index_new_map.push_back(0); // first map always starts at zero-index two qubit gate


		//build ZDD that represents all swaps that can be done in parallel 
		zdd_base zdd_swap_layers(arch_.edges.size());
		zdd_swap_layers.build_tautologies();
		
		auto univ_fam = zdd_swap_layers.tautology();
		std::vector<zdd_base::node> edges_p;

		std::vector<std::vector<uint8_t>> incidents(arch_.num_vertices);
		for (auto i = 0u; i < arch_.edges.size(); ++i) {
			incidents[arch_.edges[i].first].push_back(i);
			incidents[arch_.edges[i].second].push_back(i);
		}

		for (auto& i : incidents) {
			std::sort(i.rbegin(), i.rend());
			auto set = zdd_swap_layers.bot();
			for (auto o : i) {
				set = zdd_swap_layers.union_(set, zdd_swap_layers.elementary(o));
			}
			edges_p.push_back(set);
		}
		
		auto edges_union = zdd_swap_layers.bot();
		for (int v = circ_.num_qubits() - 1; v >= 0; --v) {
			edges_union = zdd_swap_layers.union_(edges_union, zdd_swap_layers.choose(edges_p[v], 2));
		}

		auto layers = zdd_swap_layers.nonsupersets(univ_fam, edges_union);


		std::vector< std::vector<uint32_t>> *set_vector = new std::vector< std::vector<uint32_t>>();
		zdd_swap_layers.sets_to_vector(layers,set_vector);
		global_swap_layers = *set_vector;
		delete set_vector;

                auto swap_layers_built = std::chrono::system_clock::now()-start; //swap layer zdd built

        	//below is where we look for maps!
		circ_.foreach_cgate([&](auto const& n) {
			if (n.gate.is_double_qubit()){
				n.gate.foreach_control([&](auto _c) { c = _c; });
				n.gate.foreach_target([&](auto _t) { t = _t; });
				if (m == zdd_.bot()){
					/* first gate */
					m = map(c, t);
					zdd_.ref(m);
				}
                		else{
					auto m_next = map(c, t);
					if (auto mp = zdd_.nonsupersets(zdd_.join(m, m_next), bad_); mp == zdd_.bot()){
						std::vector<uint32_t> new_mappings_cnt(global_swap_layers.size(),0);
                        			std::vector<uint32_t> depth_count(global_swap_layers.size(),0);
						std::vector<uint32_t> swap_count(global_swap_layers.size(),0);

						for(uint32_t i = 0; i < global_swap_layers.size(); i++){
							swap_count[i] = global_swap_layers[i].size();
							for(auto const& item : global_swap_layers[i]){
								std::swap(edge_perm_[arch_.edges[item].first], edge_perm_[arch_.edges[item].second]);
                            					zdd_.deref(valid_);
								zdd_.garbage_collect();
                            					init_valid();
							}
							auto m_next = map(c, t);
                            				if (auto mp = zdd_.nonsupersets(zdd_.join(m, m_next), bad_); mp == zdd_.bot()){
                                				//cannot extend map
                                				new_mappings_cnt[i] = 0;
                                				depth_count[i] = 0;
                                				//unswap physical qubits for next iteration
                                				for(auto const& item : global_swap_layers[i]){
									std::swap(edge_perm_[arch_.edges[item].first], edge_perm_[arch_.edges[item].second]);
                            						zdd_.deref(valid_);
									zdd_.garbage_collect();
                            						init_valid();
								}
                                				continue;
							}
							else{
                                				//can extend map. determine depth and number of mappings
                                				//move on to next possible swap
                                				//make sure to swap qubits back before next iteration
                                
                                				auto m_prime = m;
                                
                                				uint32_t ctr2 = 0;
                                				uint32_t cc, tt;
                                				uint32_t single_depth = 0;
                                				uint32_t end_found = 0;
                                
                                				circ_.foreach_cgate([&](auto const& nn) {
                                    					if (nn.gate.is_double_qubit()){
                                        					if (ctr2>=ctr && end_found != 1){
                                            						nn.gate.foreach_control([&](auto _c) { cc = _c; });
                                            						nn.gate.foreach_target([&](auto _t) { tt = _t; });
                                            						auto m_next_prime = map(cc, tt);
                                            						if (auto mp_prime = zdd_.nonsupersets(zdd_.join(m_prime, m_next_prime), bad_); mp_prime == zdd_.bot()){
                                                						depth_count[i] = single_depth;
                                                						end_found = 1;
                                            						}
											else{
                                                						m_prime = mp_prime;
                                                						single_depth++;
                                            						}
                                            
                                            
                                        					}
                                        					++ctr2;
                                    					}
                                				});
                                				new_mappings_cnt[i] = zdd_.count_sets(mp);
                                				for(auto const& item : global_swap_layers[i]){
									std::swap(edge_perm_[arch_.edges[item].first], edge_perm_[arch_.edges[item].second]);
                            						zdd_.deref(valid_);
									zdd_.garbage_collect();
                            						init_valid();
								}
                                				continue;

                            				}

						}
                        			//USE BELOW TO SET SCORE for layers
                        			std::vector<double> scores(global_swap_layers.size(),0);
                        
                        			for(uint32_t index = 0; index < depth_count.size(); index ++){
                            				double inv_swap_cnt;
                            				if (swap_count[index] == 0){
								inv_swap_cnt = 0;
							}
							else{
								inv_swap_cnt = 1.0/swap_count[index];
							}
							scores[index] = (depth_count[index]*depth_weight + new_mappings_cnt[index]*map_weight)*((inv_swap_cnt)*swap_weight);


							//see metrics used to pick swap(s)
                                                        if(ps_.verbose){
                                                               std::cout << index << ": depth - " << depth_count[index] << " | mappings - " << new_mappings_cnt[index] << " | swap_count - " << swap_count[index]<< " | score: " << scores[index]<< "\n"; 
                                                        }

                        			}
                        
                        			uint32_t max_index = std::max_element(scores.begin(), scores.end())-scores.begin();
                        			int max_score = scores[max_index];

                        			if (max_score == 0){
                                                        if(ps_.verbose){
                            				        std::cout << "Metrics before partition end :\n";
                            				        std::cout << "\nTotal SWAPs: " << swapped_qubits.size() << "\n";
                            				        for(uint32_t i = 0; i<swapped_qubits.size(); i++ ){
                                				        std::cout << "Swap at gate: " <<index_of_swap[i] <<" | Physical qubits swapped: " << std::string(1, 'A' + swapped_qubits[i][0])<< " " <<std::string(1, 'A' + swapped_qubits[i][1])<< "\n";
                            				        }
                                                        }

                                                        zdd_.ref(m);
                                                        mappings.push_back(m);
                                                        //m = zdd.bot(); // m_next;
                                                        //zdd_.ref(m);
                                                        //zdd_.garbage_collect();
                                                        //zdd_.deref(m);
                                                        index_new_map.push_back(ctr);

                                                        std::iota(edge_perm_.begin(), edge_perm_.end(), 0u);
                                                        zdd_.deref(valid_);
                                                        zdd_.garbage_collect();
                                                        init_valid();
                                                        m = map(c, t);
                                                        zdd_.ref(m);

                        			}
                        			else{
                            				for(auto const& item : global_swap_layers[max_index]){
								std::swap(edge_perm_[arch_.edges[item].first], edge_perm_[arch_.edges[item].second]);
                            					zdd_.deref(valid_);
								zdd_.garbage_collect();
                            					init_valid();
							}
                               
							auto m_next = map(c, t);
                            				mp = zdd_.nonsupersets(zdd_.join(m, m_next), bad_);
							zdd_.deref(m);
							zdd_.ref(mp);
							m = mp;
							for(auto const& item : global_swap_layers[max_index]){
								std::vector<uint32_t> one_swap;
								one_swap.push_back(arch_.edges[item].first);
                            					one_swap.push_back(arch_.edges[item].second);
                            					swapped_qubits.push_back(one_swap);
								index_of_swap.push_back(ctr);
								
							}

                        			}

                        
					}
                    			else{
						zdd_.deref(m);
						zdd_.ref(mp);
						m = mp;
					}
				}
				++ctr;
			}
		});
		
                auto maps_found = std::chrono::system_clock::now() - swap_layers_built - start; //sets for maps found
                
                zdd_.ref(m);
		mappings.push_back(m);
        
        	if(ps_.verbose){
                        std::cout << "\nTotal SWAPs: " << swapped_qubits.size() << "\n";
        	        for(uint32_t i = 0; i<swapped_qubits.size(); i++ ){
            		        std::cout << "Swap at gate: " <<index_of_swap[i] <<" | Physical qubits swapped: " << std::string(1, 'A' + swapped_qubits[i][0])<< " " <<std::string(1, 'A' + swapped_qubits[i][1])<< "\n";
                        }
            
        	}
                


                //holds the size of each map
                std::vector<uint32_t> map_coverage;
                for(uint32_t i = 0; i<index_new_map.size(); i++ ){
                        if(ps_.verbose){
                                std::cout << "gate index " << index_new_map[i] << "\n";
                        }
                        if( i == index_new_map.size()-1){
                                map_coverage.push_back(ctr-index_new_map[i]);
                                if(ps_.verbose){
                                        std::cout << "size " << ctr-index_new_map[i] << "\n";
                                }
                        }
                        else{
                                map_coverage.push_back(index_new_map[i+1]-index_new_map[i]);
                                if(ps_.verbose){
                                        std::cout << "size " << index_new_map[i+1]-index_new_map[i] << "\n";
                                }
                        }
                        
                }
                

                uint32_t loc_max_map_coverage = std::max_element(map_coverage.begin(), map_coverage.end())-map_coverage.begin();
                uint32_t partition_start;
                for(uint32_t i = 0; i<index_of_swap.size(); i++ ){
                        if(index_of_swap[i]>=index_new_map[loc_max_map_coverage]){
                                partition_start = i;
                                break;
                        }
                }

        	//retrieve all sets from largest partition to pick one for new map
                std::vector< std::vector<uint32_t>> *set_vector2 = new std::vector< std::vector<uint32_t>>();
                zdd_.sets_to_vector(mappings[loc_max_map_coverage],set_vector2);
                global_found_sets = *set_vector2;
                delete set_vector2;

        
		// THIS BELOW CHOOSES THE SET TO MAP NEW CIRCUIT TO!
        	uint32_t set_to_use = 0;

        	std::vector <uint32_t> used_phys_qubits;
                std::vector <int> chosen_mapping(circ_.num_qubits(),-1); //index is pseudo, what is stored is the mapping
        	for(auto const& item:global_found_sets[set_to_use]){
            		uint32_t pseudo_qubit = item /circ_.num_qubits();
            		uint32_t physical_qubit = item % circ_.num_qubits();
            		chosen_mapping[pseudo_qubit] = physical_qubit;
                        used_phys_qubits.push_back(physical_qubit);
        	}


                std::vector <uint32_t> unused_phys_qubits;
                for(int i = 0; i< chosen_mapping.size() ;i++){
                        if(std::find(used_phys_qubits.begin(), used_phys_qubits.end(), i) == used_phys_qubits.end()){
                                unused_phys_qubits.push_back(i);
                        }
                }

                uint32_t qubit_ctr = 0;
                for(int i = 0; i< chosen_mapping.size() ;i++){
                        if(chosen_mapping[i]==-1){
                                chosen_mapping[i] = unused_phys_qubits[qubit_ctr];
                                qubit_ctr++;
                        }
                }

       		std::vector <int> current_mapping = chosen_mapping;


        	//make new circuit here
        	using namespace tweedledum;

		netlist<mcst_gate> network2;
		uint32_t network2_volume = 0;
		std::vector<uint32_t> network2_depth(circ_.num_qubits(),0);
		uint32_t q2_gate_count = 0;

        	for(uint32_t i = 0; i< circ_.num_qubits(); i++){
            		network2.add_qubit();
        	}
        
        	ctr = 0;
        	uint32_t index_counter = partition_start;//start at first 2q gate in partition
        	circ_.foreach_cgate([&](auto const& n) {
            		if (n.gate.is_double_qubit()){
                		//keep track and include SWAPS
                		n.gate.foreach_control([&](auto _c) { c = _c; });
                		n.gate.foreach_target([&](auto _t) { t = _t; });
                		//std::cout <<std::string(1, 'a' + c)  << " " << std::string(1, 'a' + t) << "\n";
                                
                                //need condition below to prevent seg faults if no swaps are required for circuit
                		if(index_of_swap.size() !=0){
                                        //if 2q gate is one that needs swap, and its in the correct partition range, add it!
                                        if(ctr == index_of_swap[index_counter] && (ctr < (index_new_map[loc_max_map_coverage]+ map_coverage[loc_max_map_coverage] ))){
					        //sometimes multiple swaps from layers are needed...
                                                while(ctr == index_of_swap[index_counter]){
						        //insert as many swaps that are needed in a particular spot
						        network2.add_gate(gate::cx,qubit_id(swapped_qubits[index_counter][0]),qubit_id(swapped_qubits[index_counter][1]));
						        network2.add_gate(gate::cx,qubit_id(swapped_qubits[index_counter][1]),qubit_id(swapped_qubits[index_counter][0]));
						        network2.add_gate(gate::cx,qubit_id(swapped_qubits[index_counter][0]),qubit_id(swapped_qubits[index_counter][1]));

						        network2_volume = network2_volume + 3;
						        network2_depth[swapped_qubits[index_counter][0]] = network2_depth[swapped_qubits[index_counter][0]] + 3;
						        network2_depth[swapped_qubits[index_counter][1]] = network2_depth[swapped_qubits[index_counter][1]] + 3;
						        q2_gate_count= q2_gate_count + 3;
                	    			        
                                                        //adjust qubits in current mapping
                	    			        auto itr0 = std::find(current_mapping.begin(), current_mapping.end(), swapped_qubits[index_counter][0]);
                	    			        auto itr1 = std::find(current_mapping.begin(), current_mapping.end(), swapped_qubits[index_counter][1]);
	
                	    			        uint32_t indx0 = std::distance(current_mapping.begin(),itr0);
                	    			        uint32_t indx1 = std::distance(current_mapping.begin(),itr1);
	
                	    			        current_mapping[indx0]= swapped_qubits[index_counter][1];
                	    			        current_mapping[indx1]= swapped_qubits[index_counter][0];

                	    			        
	
                	    			        index_counter++;

                                                

					        }
                                                //insert gate
                	    			network2.add_gate(n.gate,qubit_id(current_mapping[c]),qubit_id(current_mapping[t]));
						network2_volume++;
						network2_depth[current_mapping[c]]= network2_depth[current_mapping[c]]+1;
						network2_depth[current_mapping[t]] = network2_depth[current_mapping[t]]+1;
						q2_gate_count++;

                		        }
                		        else{
                	    		        //insert gate with fixed qubits
                	    		        network2.add_gate(n.gate,qubit_id(current_mapping[c]),qubit_id(current_mapping[t]));
					        network2_volume++;
					        network2_depth[current_mapping[c]]= network2_depth[current_mapping[c]]+1;
					        network2_depth[current_mapping[t]] = network2_depth[current_mapping[t]]+1;
					        q2_gate_count++;
	
                		        }

                                }
                                else{
                                        //no swaps needed for circuit b/c zero items in index_of_swap
                                        network2.add_gate(n.gate,qubit_id(current_mapping[c]),qubit_id(current_mapping[t]));
					network2_volume++;
					network2_depth[current_mapping[c]]= network2_depth[current_mapping[c]]+1;
					network2_depth[current_mapping[t]] = network2_depth[current_mapping[t]]+1;
					q2_gate_count++;
                                        
                                }
                		ctr++;
            		}
            		else{
                		//write gate
				n.gate.foreach_target([&](auto _t) { t = _t; });
				network2.add_gate(n.gate,qubit_id(current_mapping[t]));
				network2_volume++;
				network2_depth[current_mapping[t]] = network2_depth[current_mapping[t]]+1;
            		}

        	});
		if(network2_volume < 40)
		{
			write_unicode(network2);
		}

                auto new_crk_made = std::chrono::system_clock::now() - maps_found - swap_layers_built - start; //circuit made

		uint32_t max_depth_index = std::max_element(network2_depth.begin(), network2_depth.end())-network2_depth.begin();
        	uint32_t max_depth = network2_depth[max_depth_index];
                if(ps_.verbose){
                        std::cout <<"DEPTH: "<< max_depth<< " | VOL.: " << network2_volume << " | 2Q GATE COUNT: " << q2_gate_count <<"\n";
                }

                std::string file_name = ps_.mapped_filename;
                write_quil(network2,file_name);
                std::ofstream ckt_file;
                ckt_file.open(file_name,std::ios_base::app);
                ckt_file << "#DEPTH: "<< max_depth<< " | VOL.: " << network2_volume << " | 2Q GATE COUNT: " << q2_gate_count <<"\n";
                ckt_file <<"#CHOSEN MAPPING :";
                for (auto const& item : chosen_mapping){
                        ckt_file << item << " ";
                }
                ckt_file <<"\n";
                ckt_file <<"#LAST CURRENT MAPPING :";
                for (auto const& item : current_mapping){
                        ckt_file << item << " ";
                }
                ckt_file <<"\n";

		uint32_t total = 0;
                if(ps_.verbose){
                       std::cout<< "\n";
		        for (auto const& map : mappings) {
			        std::cout << "found mapping with " << zdd_.count_sets(map)<< " mappings using " << zdd_.count_nodes(map) << " nodes.\n";
			        total += zdd_.count_sets(map);
      
            		        //below prints the found mappings in partition
           		        std::cout << "\nfound sets: \n";
            		        zdd_.print_sets(map, fmt_);
            		        std::cout << "\n";
            
			        zdd_.deref(map);
		        }
		        zdd_.summary();
		        std::cout << "Total mappings: " << total << "\n"; 
                }
                if(ps_.verbose==false){
		        for (auto const& map : mappings) {
			        zdd_.deref(map);
		        }
                }
        	

		zdd_.deref(valid_);
		zdd_.deref(bad_);
		for (auto& f : from_)
			zdd_.deref(f);
		zdd_.garbage_collect();
		// zdd_.debug();

                auto end_time = std::chrono::system_clock::now()-start;

                std::chrono::duration<double> swap_layers_built_sec = swap_layers_built;
                std::chrono::duration<double> maps_found_sec = maps_found;
                std::chrono::duration<double> new_crk_made_sec = new_crk_made;
                std::chrono::duration<double> end_time_sec = end_time;

                st_.time_swap_layers_built = swap_layers_built_sec.count();
                st_.time_map_search = maps_found_sec.count();
                st_.time_new_crk = new_crk_made_sec.count();
                st_.time_total = end_time_sec.count();
                
                std::vector<uint32_t> u_chosen_mapping;
                for(auto i : chosen_mapping)
                        u_chosen_mapping.push_back(i);
                create_pathsum(circ_, network2, u_chosen_mapping);


                
                
	}

private:
	auto index(uint32_t v, uint32_t p) const
	{
		// return p * circ_.num_qubits() + v;
		return v * arch_.num_vertices + p;
	}

	void init_from()
	{
		for (auto v = 0u; v < circ_.num_qubits(); ++v) {
			auto set = zdd_.bot();
			for (int p = arch_.num_vertices - 1; p >= 0; --p) {
				set = zdd_.union_(set, zdd_.elementary(index(v, p)));
			}
			from_[v] = set;
			zdd_.ref(set);
		}
	}

	void init_to()
	{
		for (auto p = 0u; p < arch_.num_vertices; ++p) {
			auto set = zdd_.bot();
			for (int v = circ_.num_qubits() - 1; v >= 0; --v) {
				set = zdd_.union_(set, zdd_.elementary(index(v, p)));
			}
			to_[p] = set;
			zdd_.ref(set);
		}
	}

	void init_valid()
	{
		valid_ = zdd_.bot();
		for (auto const& [p, q] : arch_.edges) {
			valid_ = zdd_.union_(valid_, zdd_.join(to_[edge_perm_[p]], to_[edge_perm_[q]]));
		}
		zdd_.ref(valid_);

	}

	void init_bad()
	{
		bad_ = zdd_.bot();
		for (int v = circ_.num_qubits() - 1; v >= 0; --v) {
			bad_ = zdd_.union_(bad_, zdd_.choose(from_[v], 2));
		}
		for (int p = arch_.num_vertices - 1; p >= 0; --p) {
			bad_ = zdd_.union_(bad_, zdd_.choose(to_[p], 2));
		}
		zdd_.ref(bad_);
	}

	zdd_base::node map(uint32_t c, uint32_t t)
	{
		return zdd_.intersection(zdd_.join(from_[c], from_[t]), valid_);
	}

        void create_pathsum(netlist<mcst_gate> original_nwk, netlist<mcst_gate> new_nwk, std::vector<uint32_t>& map){
                netlist<mcst_gate> original_nwk_cx;
                netlist<mcst_gate> new_nwk_cx;
                uint32_t c, t;
                for(uint32_t i = 0; i< original_nwk.num_qubits(); i++){
            		original_nwk_cx.add_qubit();
                        new_nwk_cx.add_qubit();
        	}
                original_nwk.foreach_cgate([&](auto const& n) {
                        if (n.gate.is_double_qubit()){
                                n.gate.foreach_control([&](auto _c) { c = _c; });
                	        n.gate.foreach_target([&](auto _t) { t = _t; });
                                original_nwk_cx.add_gate(n.gate,qubit_id(c),qubit_id(t));
                        }

                });
                new_nwk.foreach_cgate([&](auto const& n) {
                        if (n.gate.is_double_qubit()){
                                n.gate.foreach_control([&](auto _c) { c = _c; });
                	        n.gate.foreach_target([&](auto _t) { t = _t; });
                                new_nwk_cx.add_gate(n.gate,qubit_id(c),qubit_id(t));
                        }

                });

	        std::vector<uint32_t> dummy(map.size(), 0);
	        std::iota(dummy.begin(), dummy.end(), 0);
                pathsum_view sums(original_nwk_cx, dummy);
                pathsum_view sums2(new_nwk_cx, map);

                if(ps_.verbose){
                        std::cout << "pathsums original network: \n";
	                sums.foreach_coutput([&](auto const& node) {
		                auto& sum = sums.get_pathsum(node);
		                for (auto e : sum) {
			                std::cout << e << ' ';
		                }
		                std::cout << '\n';
	                });

                        std::cout << "pathsums mapped network: \n";
	                sums2.foreach_coutput([&](auto const& node) {
		                auto& sum2 = sums2.get_pathsum(node);
		                for (auto e : sum2){
			                std::cout << e << ' ';
		                }
		                std::cout << '\n';
	                });

                
                }
	        auto num_ok = 0;
	        sums.foreach_coutput([&](auto const& node) {
		        auto& sum = sums.get_pathsum(node);
		        sums2.foreach_coutput([&](auto const& node) {
			        auto& sum2 = sums2.get_pathsum(node);
			        if (sum == sum2) {
				        num_ok++;
			        }
		        });
	        });
	        //std::cout << "num: " << num_ok << '\n';
	        assert(num_ok == sums.num_qubits());



        }

private:
	Ntk const& circ_;
	device const& arch_;
        find_maximal_partitions_params const& ps_;
        find_maximal_partitions_stats& st_;

	zdd_base zdd_;
	std::vector<zdd_base::node> from_, to_;
	zdd_base::node valid_, bad_;
	std::vector<uint32_t> edge_perm_;

	struct set_formatter_t {
		set_formatter_t(uint32_t n)
		    : n(n)
		{}
		std::string operator()(uint32_t v) const
		{
            //return std::string(1, 'a' + (v % n)) + std::string(1, 'A' + (v / n));
			return std::string(1, 'a' + (v / n)) + std::string(1, 'A' + (v % n));
		}

	private:
		uint32_t n;
	} fmt_;
};

} // namespace detail

/*! \brief
 *
 * **Required gate functions:**
 *
 * **Required network functions:**
 *
 */
template<typename Network>
Network zdd_map(Network& network, device const& arch, find_maximal_partitions_params const& ps = {},
                find_maximal_partitions_stats* pst = nullptr)
{
        find_maximal_partitions_stats st;
	detail::find_maximal_partitions_impl<Network> impl(network, arch, ps, st);
	Network mapped_network = impl.run();
	if (pst) {
                *pst = st;
        }
	return mapped_network;
}

} // namespace tweedledum
