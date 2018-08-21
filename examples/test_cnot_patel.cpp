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
	// std::vector<std::pair<uint32_t, float>> parities{
	// 													{0b0110,
	// T},
	// {0b0001, T},
	// {0b1001, T},
	// {0b0111, T},
	// {0b1011, T},
	// {0b0011, T}}
	//   												};


	dag_path<qc_gate> network;

	//std::vector<uint32_t> matrix{0b110000, 0b100110, 0b010010, 0b111111, 0b110111, 0b001110};
	//std::vector<uint32_t> matrix {0b000011,0b011001,0b010010,0b111111,0b111011,0b011100};
	//std::vector<uint32_t> matrix{0b011011,0b011101,0b101000,0b111010,0b111110,0b011000 };
	
	//std::vector<uint32_t> matrix{0b1010, 0b0100, 0b0110, 0b0101};
	std::vector<uint32_t> matrix{0b1111,0b0110,0b0100,0b1000};

	//std::vector<uint32_t> p1{0b1111,0b0110,0b0100,0b1000};

	float T = 0.393;
	std::vector<float> Ts{T, T, T, T, T, T};

	//gray_synth(network, 4, p1, Ts);
	cnot_patel(network,matrix,2);

	write_quil(network, std::cout);
}
