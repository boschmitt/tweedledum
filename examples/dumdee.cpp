/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt < bruno [at] oschmitt [dot] com >
*-----------------------------------------------------------------------------*/
#include <cstdlib>
#include <iostream>
#include <tweedledum/io/dotqc.hpp>
#include <tweedledum/representations/quantum_circuit.hpp>

using namespace tweedledum;

int main(int argc, char** argv)
{
	if (argc < 2) {
		std::cerr << "[e] Input file not specified.\n";
		return EXIT_FAILURE;
	}
	quantum_circuit qc;
	dotqc_reader<quantum_circuit> reader(qc);
	dotqc_read(argv[1], reader, identify_gate_kind());
	return EXIT_SUCCESS;
}
