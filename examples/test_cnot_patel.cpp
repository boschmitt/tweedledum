#include <cstdint>
#include <iostream>
#include <tweedledum/algorithms/synthesis/cnot_patel.hpp>
#include <tweedledum/io/quil.hpp>
#include <tweedledum/networks/dag_path.hpp>
#include <tweedledum/networks/gates/qc_gate.hpp>
#include <vector>

using namespace tweedledum;

int main()
{
	dag_path<qc_gate> network;
	std::vector<uint32_t> matrix{
	    {0b110000, 0b100110, 0b010010, 0b111111, 0b110111, 0b001110}};
	cnot_patel(network, matrix, 2);
	write_quil(network, std::cout);
}
