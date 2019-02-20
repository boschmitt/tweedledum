/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken, Kate Smith
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <chrono>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <random>
#include <tweedledum/algorithms/mapping/zddmap.hpp>
#include <tweedledum/algorithms/synthesis/stg.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/io/write_unicode.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>


#include <tweedledum/algorithms/decomposition/barenco.hpp>
#include <tweedledum/algorithms/synthesis/dbs.hpp>
#include <tweedledum/algorithms/synthesis/stg.hpp>
#include <tweedledum/gates/gate_set.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>

using namespace tweedledum;

device_t ring(uint8_t m)
{
	std::vector<std::pair<uint8_t, uint8_t>> edges;
	for (auto i = 0; i < m; ++i) {
		edges.emplace_back(i, (i + 1) % m);
	}
	return {edges, m};
}

device_t star(uint8_t m)
{
	std::vector<std::pair<uint8_t, uint8_t>> edges;
	for (auto i = 1; i < m; ++i) {
		edges.emplace_back(0, i);
	}
	return {edges, m};
}

device_t grid(uint8_t w, uint8_t h)
{
	std::vector<std::pair<uint8_t, uint8_t>> edges;
	for (auto x = 0; x < w; ++x) {
		for (auto y = 0; y < h; ++y) {
			auto e = y * w + x;
			if (x < w - 1) {
				edges.emplace_back(e, e + 1);
			}
			if (y < h - 1) {
				edges.emplace_back(e, e + w);
			}
		}
	}
	return {edges, static_cast<uint8_t>(w * h)};
}

device_t random(uint8_t m, uint8_t num_edges)
{
	std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<uint8_t> d1(0, m - 1);
	std::uniform_int_distribution<uint8_t> d2(1, m - 2);

	std::vector<std::pair<uint8_t, uint8_t>> edges;
	while (edges.size() != num_edges) {
		uint8_t p = d1(gen);
		uint8_t q = (p + d2(gen)) % m;
		std::pair<uint8_t, uint8_t> edge{std::min(p, q), std::max(p, q)};
		if (std::find(edges.begin(), edges.end(), edge) == edges.end()) {
			std::cout << (int) edge.first << " " << (int) edge.second << "\n";
			edges.push_back(edge);
		}
	}
        

	return {edges, m};
}

netlist<mcst_gate> make_network_from_quil(std::string file_name){
        //use this to transform quil to netlist
        //note: quil must be in clifford+t representation
        netlist<mcst_gate> new_nwk;
        std::vector<std::string> qubits_used;

        std::ifstream ckt_file(file_name);
        if(ckt_file.is_open()){
                std::string line;
                while(getline(ckt_file,line)){
                        std::istringstream buf(line);
                        std::istream_iterator<std::string> beg(buf), end;
                        std::vector<std::string> tokens(beg, end);
                        if(tokens[0]=="H"){
                                if(std::find(qubits_used.begin(), qubits_used.end(), tokens[1]) != qubits_used.end()){
                                        new_nwk.add_gate(gate::hadamard,tokens[1]);
                                }
                                else{
                                        qubits_used.push_back(tokens[1]);
                                        new_nwk.add_qubit(tokens[1]);
                                        new_nwk.add_gate(gate::hadamard,tokens[1]);
                                }
                        }
                        else if(tokens[0]=="X"){
                                if(std::find(qubits_used.begin(), qubits_used.end(), tokens[1]) != qubits_used.end()){
                                        new_nwk.add_gate(gate::pauli_x,tokens[1]);
                                }
                                else{
                                        qubits_used.push_back(tokens[1]);
                                        new_nwk.add_qubit(tokens[1]);
                                        new_nwk.add_gate(gate::pauli_x,tokens[1]);
                                }
                        }
                        else if(tokens[0]=="Z"){
                                if(std::find(qubits_used.begin(), qubits_used.end(), tokens[1]) != qubits_used.end()){
                                        new_nwk.add_gate(gate::pauli_z,tokens[1]);
                                }
                                else{
                                        qubits_used.push_back(tokens[1]);
                                        new_nwk.add_qubit(tokens[1]);
                                        new_nwk.add_gate(gate::pauli_z,tokens[1]);
                                }
                        }
                        else if(tokens[0]=="T"){
                                if(std::find(qubits_used.begin(), qubits_used.end(), tokens[1]) != qubits_used.end()){
                                        new_nwk.add_gate(gate::t,tokens[1]);
                                }
                                else{
                                        qubits_used.push_back(tokens[1]);
                                        new_nwk.add_qubit(tokens[1]);
                                        new_nwk.add_gate(gate::t,tokens[1]);
                                }
                        }
                        else if(tokens[0]=="RZ(-pi/4)"){
                                if(std::find(qubits_used.begin(), qubits_used.end(), tokens[1]) != qubits_used.end()){
                                        new_nwk.add_gate(gate::t_dagger,tokens[1]);
                                }
                                else{
                                        qubits_used.push_back(tokens[1]);
                                        new_nwk.add_qubit(tokens[1]);
                                        new_nwk.add_gate(gate::t_dagger,tokens[1]);
                                }
                        }
                        else if(tokens[0]=="S"){
                                if(std::find(qubits_used.begin(), qubits_used.end(), tokens[1]) != qubits_used.end()){
                                        new_nwk.add_gate(gate::phase,tokens[1]);
                                }
                                else{
                                        qubits_used.push_back(tokens[1]);
                                        new_nwk.add_qubit(tokens[1]);
                                        new_nwk.add_gate(gate::phase,tokens[1]);
                                }
                        }
                        else if(tokens[0]=="RZ(-pi/2)"){
                                if(std::find(qubits_used.begin(), qubits_used.end(), tokens[1]) != qubits_used.end()){
                                        new_nwk.add_gate(gate::phase_dagger,tokens[1]);
                                }
                                else{
                                        qubits_used.push_back(tokens[1]);
                                        new_nwk.add_qubit(tokens[1]);
                                        new_nwk.add_gate(gate::phase_dagger,tokens[1]);
                                }
                        }
                        else if(tokens[0]=="CNOT"){
                                if(std::find(qubits_used.begin(), qubits_used.end(), tokens[1]) != qubits_used.end()){
                                        if (std::find(qubits_used.begin(), qubits_used.end(), tokens[2]) != qubits_used.end()) {
                                                new_nwk.add_gate(gate::cx,tokens[1],tokens[2]);

                                        }
                                        else{
                                                qubits_used.push_back(tokens[2]);
                                                new_nwk.add_qubit(tokens[2]);
                                                new_nwk.add_gate(gate::cx,tokens[1],tokens[2]);
                                        }
                    
                                }
                                else{
                                        if (std::find(qubits_used.begin(), qubits_used.end(), tokens[2]) != qubits_used.end()){
                                                qubits_used.push_back(tokens[1]);
                                                new_nwk.add_qubit(tokens[1]);
                                                new_nwk.add_gate(gate::cx,tokens[1],tokens[2]);
                                        }
                                        else{
                                                qubits_used.push_back(tokens[1]);
                                                qubits_used.push_back(tokens[2]);
                                                new_nwk.add_qubit(tokens[1]);
                                                new_nwk.add_qubit(tokens[2]);
                                                new_nwk.add_gate(gate::cx,tokens[1],tokens[2]);
                                        }

                                }

                        }
                        else if(tokens[0]=="CZ"){
                                if(std::find(qubits_used.begin(), qubits_used.end(), tokens[1]) != qubits_used.end()){
                                        if (std::find(qubits_used.begin(), qubits_used.end(), tokens[2]) != qubits_used.end()) {
                                                new_nwk.add_gate(gate::cz,tokens[1],tokens[2]);

                                        }
                                        else{
                                                qubits_used.push_back(tokens[2]);
                                                new_nwk.add_qubit(tokens[2]);
                                                new_nwk.add_gate(gate::cz,tokens[1],tokens[2]);
                                        }
                    
                                }
                                else{
                                        if (std::find(qubits_used.begin(), qubits_used.end(), tokens[2]) != qubits_used.end()){
                                                qubits_used.push_back(tokens[1]);
                                                new_nwk.add_qubit(tokens[1]);
                                                new_nwk.add_gate(gate::cz,tokens[1],tokens[2]);
                                        }
                                        else{
                                                qubits_used.push_back(tokens[1]);
                                                qubits_used.push_back(tokens[2]);
                                                new_nwk.add_qubit(tokens[1]);
                                                new_nwk.add_qubit(tokens[2]);
                                                new_nwk.add_gate(gate::cz,tokens[1],tokens[2]);
                                        }

                                }
            
                        }
                        else{
                                std::cout << "Gate could not be added to network. Exiting...\n";
                                std::exit(0);
                        }

                }


         

        }
   

   
   return new_nwk;
}

TEST_CASE("ORIGINAL Paper example for ZDD mapper", "[zddmap]")
{
    using namespace tweedledum;
    netlist<mcst_gate> network;
    network.add_qubit();
    network.add_qubit();
    network.add_qubit();
    network.add_qubit();

    network.add_gate(gate::cz, 0, 1);
    network.add_gate(gate::cz, 1, 2);
    network.add_gate(gate::cz, 1, 3);



    write_unicode(network);

    find_maximal_partitions(network, ring(network.num_qubits()));
}

TEST_CASE("Test reading in quil", "[zddmap]"){
        std::string bench_name = "../examples/quil_benchmarks/tof_5.quil";
        netlist<mcst_gate> network = make_network_from_quil(bench_name);
        //write_unicode(network);
        std::cout << "number of qubits :" << network.num_qubits();
        find_maximal_partitions(network, ring(network.num_qubits()));




}






//TEST_CASE("Playing with ZDDs", "[zddmap]")
//{
//  zdd_base z(4);
//  z.debug();
//
//  /*auto f = z.union_(z.elementary(1), z.elementary(2));
//  z.ref(f);
//  std::cout << f<< "\n";
//
//  z.debug();
//
//  z.garbage_collect();
//  std::cout << "after GC\n";
//  z.debug();*/
//
//  auto f = z.union_(z.elementary(3), z.elementary(2));
//  f = z.union_(f, z.elementary(1));
//  f = z.union_(f, z.elementary(0));
//
//  auto g = z.choose(f, 2);
//  std::cout << z.count_sets(g) << "\n";
//
//  z.debug();
//}
//
//TEST_CASE("Playing with ZDD CHOOSE operator", "[zddmap]")
//{
//  zdd_base z(100);
//
//  auto f = z.elementary(99);
//  for (auto i = 98; i >= 0; --i)
//  {
//    f = z.union_(f, z.elementary(i));
//  }
//
//  auto g = z.choose(f, 50);
//  std::cout << z.count_nodes(g) << " " << z.count_sets(g) << "\n";
//  //z.debug();
//
//
//}

//TEST_CASE("ZDD Practice ", "[zddmap]")
//{
//    zdd_base z1(20);
//    zdd_base z2(3);
//    z1.debug();
//    z2.debug();
//
//    auto f = z1.union_(z1.elementary(0),z1.elementary(1));
//    z1.debug();
//    auto gg = z1.union_(f,z1.elementary(2));
//    auto g = z1.join(gg,z1.elementary(1));
//    auto h = z1.intersection(g,z1.join(z1.elementary(0),z1.elementary(1)));
//    auto hh = z1.join(h,g);
//    z1.debug();
//    //z1.print_sets(f);
//    std::cout <<"one \n";
//    z1.print_sets(g);
//    std::cout <<"two \n";
//    z1.print_sets(f);
//    std::cout <<"three \n";
//    z1.print_sets(gg);
//    std::cout<<"four \n";
//    z1.print_sets(h);
//    std::cout <<"five \n";
//    z1.print_sets(hh);
//
//
//}


// TEST_CASE("Extend paper example for ZDD mapper", "[zddmap]")
// {
//     using namespace tweedledum;
//     netlist<mcst_gate> network;
//     network.add_qubit();
//     network.add_qubit();
//     network.add_qubit();
//     network.add_qubit();
    


//     network.add_gate(gate::cz, 0, 1);
//     network.add_gate(gate::cz, 1, 2);
//     network.add_gate(gate::cz, 1, 3);

//     network.add_gate(gate::cz, 2,3);


//     network.add_gate(gate::cz, 0, 1);
//     network.add_gate(gate::cz, 1, 2);
//     network.add_gate(gate::cz, 1, 3);

//     network.add_gate(gate::cz, 2,3);

//     network.add_gate(gate::cz, 0, 1);
//     network.add_gate(gate::cz, 3, 2);
//     network.add_gate(gate::cz, 1, 3);

//     network.add_gate(gate::cz, 2,3);


//     write_unicode(network);


//     find_maximal_partitions(network, ring(network.num_qubits()));
// }

// TEST_CASE("Extend paper example #3 for ZDD mapper", "[zddmap]")
// {
//     using namespace tweedledum;
//     netlist<mcst_gate> network;
//     network.add_qubit();
//     network.add_qubit();
//     network.add_qubit();
//     network.add_qubit();
    
    
    
//     network.add_gate(gate::cz, 0, 1);
//     network.add_gate(gate::cz, 1, 2);
//     network.add_gate(gate::cz, 1, 3);
    
//     network.add_gate(gate::cz, 2,3);
    
    
//     network.add_gate(gate::cz, 0, 1);
//     network.add_gate(gate::cz, 1, 2);
//     network.add_gate(gate::cz, 1, 3);
    
//     network.add_gate(gate::cz, 2,3);
    
//     network.add_gate(gate::cz, 0, 1);
//     network.add_gate(gate::cz, 3, 2);
//     network.add_gate(gate::cz, 1, 3);
    
//     network.add_gate(gate::cz, 2,3);
    
//     network.add_gate(gate::cz, 3,2);
//     network.add_gate(gate::cz, 3,1);
//     network.add_gate(gate::cz, 3,0);
    
    
    
    
    
    
    
//     write_unicode(network);
    
    
//     find_maximal_partitions(network, ring(network.num_qubits()));
// }




// TEST_CASE("Extend paper example #2 for ZDD mapper", "[zddmap]")
// {
//    using namespace tweedledum;
//    netlist<mcst_gate> network;
//    network.add_qubit();
//    network.add_qubit();
//    network.add_qubit();
//    network.add_qubit();
//    network.add_qubit();
//    network.add_qubit();
//    network.add_qubit();
//    network.add_qubit();


//    network.add_gate(gate::cz, 0, 1);
//    network.add_gate(gate::cz, 1, 2);
//    network.add_gate(gate::cz, 1, 3);

//    network.add_gate(gate::cz, 2,5);


//    network.add_gate(gate::cz, 0, 1);
//    network.add_gate(gate::cz, 1, 2);
//    network.add_gate(gate::cz, 1, 3);






//    write_unicode(network);


//    find_maximal_partitions(network, ring(network.num_qubits()));
// }
// TEST_CASE("Extend paper example #3.5 for ZDD mapper", "[zddmap]")
// {
//    using namespace tweedledum;
//    netlist<mcst_gate> network;
//    network.add_qubit();
//    network.add_qubit();
//    network.add_qubit();
//    network.add_qubit();
//    network.add_qubit();
//    network.add_qubit();
//    network.add_qubit();
//    network.add_qubit();
//    network.add_qubit();
//    network.add_qubit();




//    network.add_gate(gate::cz, 0, 1);
//    network.add_gate(gate::cz, 1, 2);
//    network.add_gate(gate::cz, 1, 3);

//    network.add_gate(gate::cz, 2,5);


//    network.add_gate(gate::cz, 9, 8);
//    network.add_gate(gate::cz, 1, 5);
//    network.add_gate(gate::cz, 4, 3);

//    network.add_gate(gate::cz, 8, 7);
//    network.add_gate(gate::cz, 6, 8);
//    network.add_gate(gate::cz, 1, 3);

//    network.add_gate(gate::cz, 2,5);


//    network.add_gate(gate::cz, 0, 1);
//    network.add_gate(gate::cz, 1, 2);
//    network.add_gate(gate::cz, 1, 3);






//    write_unicode(network);


//    find_maximal_partitions(network, ring(network.num_qubits()));
// }

// TEST_CASE("Paper example #4 for ZDD mapper", "[zddmap]")
// {
//     //force other qubits besides A and B to swap here
//     using namespace tweedledum;
//     netlist<mcst_gate> network;
//     network.add_qubit();
//     network.add_qubit();
//     network.add_qubit();
//     network.add_qubit();
//     network.add_qubit();
//     network.add_qubit();
//     network.add_qubit();
//     network.add_qubit();
    
//     network.add_gate(gate::cz, 0, 1);
//     network.add_gate(gate::cz, 1, 2);
//     network.add_gate(gate::cz, 1, 3);

//     network.add_gate(gate::cz, 4, 5);
//     network.add_gate(gate::cz, 5, 6);
//     network.add_gate(gate::cz, 5, 7);
//     //network.add_gate(gate::hadamard,qubit_id(0));
//     //network.add_gate(gate::t,qubit_id(2));


    
//     write_unicode(network);
    
//     find_maximal_partitions(network, ring(network.num_qubits()));
// }

//TEST_CASE("Check DBS with PRIME(3) and spectrum", "[dbs]")
//{
//    using namespace tweedledum;
//    std::vector<uint32_t> permutation{{0, 2, 3, 5, 7, 1, 4, 6}};
//    const auto network = dbs<netlist<mcmt_gate>>(permutation, stg_from_spectrum());
//
//    CHECK(network.num_gates() == 48u);
//    CHECK(network.num_qubits() == 3u);
//    write_unicode(network);
//}


