/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt < bruno [at] oschmitt [dot] com >
*-----------------------------------------------------------------------------*/
#include <cstdlib>
#include <iostream>
#include <tweedledum/io/dotqc.hpp>
#include <tweedledum/io/write_dot.hpp>
#include <tweedledum/io/write_qpic.hpp>
#include <tweedledum/representations/quantum_circuit.hpp>

using namespace tweedledum;

int main(int argc, char** argv)
{
	if (argc < 2) {
		std::cerr << "[e] Input file not specified.\n";
		return EXIT_FAILURE;
	}
	quantum_circuit<dag_path<gate>> qc_path;
	dotqc_reader<quantum_circuit<dag_path<gate>>> reader(qc_path);
	dotqc_read(argv[1], reader, identify_gate_kind());
	write_dot(qc_path, "qc_dag_path.dot");
	write_qpic(qc_path, "qc_dat_path.qpic");
	return EXIT_SUCCESS;
}
