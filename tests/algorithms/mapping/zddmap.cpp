/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
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

TEST_CASE("Paper example for ZDD mapper", "[zddmap]")
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


TEST_CASE("Extend paper example for ZDD mapper", "[zddmap]")
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

    network.add_gate(gate::cz, 2,3);


    network.add_gate(gate::cz, 0, 1);
    network.add_gate(gate::cz, 1, 2);
    network.add_gate(gate::cz, 1, 3);

    network.add_gate(gate::cz, 2,3);

    network.add_gate(gate::cz, 0, 1);
    network.add_gate(gate::cz, 3, 2);
    network.add_gate(gate::cz, 1, 3);

    network.add_gate(gate::cz, 2,3);


    write_unicode(network);


    find_maximal_partitions(network, ring(network.num_qubits()));
}

TEST_CASE("Extend paper example #3 for ZDD mapper", "[zddmap]")
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
    
    network.add_gate(gate::cz, 2,3);
    
    
    network.add_gate(gate::cz, 0, 1);
    network.add_gate(gate::cz, 1, 2);
    network.add_gate(gate::cz, 1, 3);
    
    network.add_gate(gate::cz, 2,3);
    
    network.add_gate(gate::cz, 0, 1);
    network.add_gate(gate::cz, 3, 2);
    network.add_gate(gate::cz, 1, 3);
    
    network.add_gate(gate::cz, 2,3);
    
    network.add_gate(gate::cz, 3,2);
    network.add_gate(gate::cz, 3,1);
    network.add_gate(gate::cz, 3,0);
    
    
    
    
    
    write_unicode(network);
    
    
    find_maximal_partitions(network, ring(network.num_qubits()));
}

//TEST_CASE("Extend paper example #2 for ZDD mapper", "[zddmap]")
//{
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
//
//
//    network.add_gate(gate::cz, 0, 1);
//    network.add_gate(gate::cz, 1, 2);
//    network.add_gate(gate::cz, 1, 3);
//
//    network.add_gate(gate::cz, 2,5);
//
//
//    network.add_gate(gate::cz, 0, 1);
//    network.add_gate(gate::cz, 1, 2);
//    network.add_gate(gate::cz, 1, 3);
//
//    network.add_gate(gate::cz, 2,3);
//
//    network.add_gate(gate::cz, 0, 1);
//    network.add_gate(gate::cz, 3, 6);
//    network.add_gate(gate::cz, 1, 3);
//
//    network.add_gate(gate::cz, 2,7);
//    network.add_gate(gate::cz, 0, 4);
//    network.add_gate(gate::cz, 2, 6);
//    network.add_gate(gate::cz, 1, 3);
//
//
//    write_unicode(network);
//
//
//    find_maximal_partitions(network, ring(network.num_qubits()));
//}

TEST_CASE("Paper example #4 for ZDD mapper", "[zddmap]")
{
    //force other qubits besides A and B to swap here
    using namespace tweedledum;
    netlist<mcst_gate> network;
    network.add_qubit();
    network.add_qubit();
    network.add_qubit();
    network.add_qubit();
    network.add_qubit();
    network.add_qubit();
    network.add_qubit();
    network.add_qubit();
    
    network.add_gate(gate::cz, 0, 1);
    network.add_gate(gate::cz, 1, 2);
    network.add_gate(gate::cz, 1, 3);

    network.add_gate(gate::cz, 4, 5);
    network.add_gate(gate::cz, 5, 6);
    network.add_gate(gate::cz, 5, 7);

    
    write_unicode(network);
    
    find_maximal_partitions(network, ring(network.num_qubits()));
}

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
