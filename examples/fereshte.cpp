
#include <cstdlib>
#include <iostream>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>

#include <tweedledum/algorithms/synthesis/quantum_state_preparation.hpp>
#include <tweedledum/algorithms/generic/rewrite.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/io/qasm.hpp>
#include <tweedledum/io/write_unicode.hpp>
#include <tweedledum/networks/netlist.hpp>



int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	using namespace tweedledum;
	
	std::vector<uint32_t> prime3 = {0, 2, 3, 5, 7, 1, 4, 6};
	std::vector<uint32_t> prime4 = {0, 2, 3, 5, 7, 11, 13, 1, 4, 6, 8, 9, 10, 12, 14, 15};
	std::vector<uint32_t> prime5 = {0, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 1, 4, 6, 8, 9, 10, 12, 14, 15, 
	 16, 18, 20, 21, 22, 24, 25, 26, 27, 28, 30};
	std::vector<uint32_t> prime6 = {0, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 1, 
	4, 6, 8, 9, 10, 12, 14, 15, 16, 18, 20, 21, 22, 24, 25, 26, 27, 28, 30, 32, 33, 34, 35, 36, 38, 39, 40, 42,
	 44, 45, 46, 48, 49, 50, 51, 52, 54, 55, 56, 57, 58, 60, 62, 63};

	std::vector<uint32_t> tof3 ={0, 1, 2, 3, 4, 5, 7, 6};
	std::vector<uint32_t> tof4 ={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 14};
	std::vector<uint32_t> tof5 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 
	22, 23, 24, 25, 26, 27, 28, 29, 31, 30};
	std::vector<uint32_t> tof6 ={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
	 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 
	 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 63, 62};

	std::vector<uint32_t> hwb4 ={0, 2, 4, 12, 8, 5, 9, 11, 1, 6, 10, 13, 3, 14, 7, 15};
	std::vector<uint32_t> hwb5 = {0, 2, 4, 12, 8, 20, 24, 25, 16, 5, 9, 26, 17, 11, 19, 23, 1, 6, 10, 28, 18, 13,
	 21, 27, 3, 14, 22, 29, 7, 30, 15, 31};


	
	//std::string tt_f = "00011111";
	std::string ghz3 = "10000001"; //lsb: right side
	std::string ghz4 = "1000000000000001";
	std::string ghz5 = "10000000000000000000000000000001";
	std::string Maj3 = "11101000";
	std::string Maj4 = "1110100010000000";
    std::string tt_f = ghz3;	
	std::string bench_name = "ghz3";

    netlist<mcmt_gate> net;
	qsp_params stg;
	//stg.strategy = 0u;
	qsp<netlist<mcmt_gate>>(net,tt_f);

	netlist<mcmt_gate> net_sc;
	net.foreach_cqubit( [&]( std::string const& qlabel ){
		net_sc.add_qubit( qlabel );
	} );

	net.foreach_cgate([&](auto const& node) {
		auto const& gate = node.gate;
		if (gate.is(gate_set::mcx)) {
			if (gate.num_controls() > 1) {
				std::vector<qubit_id> q_map;
				
				gate.foreach_control([&](auto control) { q_map.push_back(control); });
				gate.foreach_target([&](auto target) { q_map.push_back(target); });
				net_sc.add_gate(gate::hadamard, q_map.back());
				unsigned nlines = q_map.size();
    			auto tt = kitty::create<kitty::dynamic_truth_table>(nlines-1);
    			kitty::set_bit(tt,pow(2,nlines-1)-1);
				detail::decomposition_mcz(net_sc,q_map,tt);
				net_sc.add_gate(gate::hadamard, q_map.back());
			}
			else
			{
				net_sc.add_gate(gate);
			}
			
		}
		else
		{
			net_sc.add_gate(gate);
		}
		
	});
	
	std::cout << "size:\n";
	std::cout<<net_sc.size()<<"\n";
	write_unicode(net_sc);
	//std::cout << "\n";
	write_qasm(net_sc,"ownfunction"+bench_name+".qasm");

	netlist<mcmt_gate> net2;
	std::vector<qubit_id> q;
	q.emplace_back(0);
	q.emplace_back(1);
	q.emplace_back(2);
	
    // for (auto i = 0u; i < 3; ++i) {
	// 	net2.add_qubit();
	// }
	// detail::decomposition_mcz<netlist<mcmt_gate>>(net2,q);
	// write_unicode(net2);
	// write_qasm(net2,"test.qasm");
	return EXIT_SUCCESS;
}
