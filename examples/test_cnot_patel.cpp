#include <cstdint>
#include <iostream>
#include <tweedledum/algorithms/synthesis/cnot_patel.hpp>
#include <tweedledum/algorithms/synthesis/gray_synth.hpp>
#include <tweedledum/io/quil.hpp>
#include <tweedledum/networks/dag_path.hpp>
#include <tweedledum/networks/gates/qc_gate.hpp>
#include <vector>

using namespace tweedledum;

int main()
{
	dag_path<qc_gate> network;

	std::vector<uint32_t> matrix{ 
									{0b110000,
									0b100110, 
									0b010010, 
									0b111111, 
									0b110111, 
									0b001110}
								};
	//cnot_patel(network, matrix, 2);
	float T = 0.393;
	// std::vector<std::pair<uint32_t, float>> parities{
	// 													{0b0110, T},
	// 													{0b0001, T},
	// 													{0b1001, T},
	// 													{0b0111, T},
	// 													{0b1011, T},
	// 													{0b0011, T}}
    //   												};

	std::vector<uint32_t> parities{
														{0b0110,
														0b0001,
														0b1001,
														0b0111,
														0b1011,
														0b0011}
      												};


	std::vector<uint32_t> my_parities{
															{0b011111,
															0b100111,
															0b100100,
															0b001010}
      													};
	std::vector<float> Ts {{T,T,T,T,T,T}};



	gray_synth(network,4,parities,Ts);

	write_quil(network, std::cout);
}
