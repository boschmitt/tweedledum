/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#include <cstdlib>
#include <iostream>
#include <tweedledum/algorithms/optimization/gate_cancellation.hpp>
#include <tweedledum/algorithms/synthesis/esop_based.hpp>
#include <tweedledum/io/dotqc.hpp>
#include <tweedledum/io/write_dot.hpp>
#include <tweedledum/io/write_qpic.hpp>
#include <tweedledum/io/quil.hpp>

#include <tweedledum/networks/gates/qc_gate.hpp>
#include <tweedledum/networks/gates/mct_gate.hpp>
#include <tweedledum/networks/dag_path.hpp>
#include <tweedledum/networks/netlist.hpp>

#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>

using namespace tweedledum;

int main(int argc, char** argv)
{
	if (argc < 2) {
		std::cerr << "[e] Input file not specified.\n";
		return EXIT_FAILURE;
	}
	dag_path<qc_gate> qc_path;
	read_quil_file(qc_path, argv[1]);
	// dotqc_reader reader(qc_path);
	// dotqc_read(argv[1], reader, identify_gate_kind());
	// write_dot(qc_path, "qc_dag_path.dot");
	// single_qubit_gate_cancellation(qc_path, false);
	// controlled_gate_cancellation(qc_path, false);
	write_qpic(qc_path, "qc_dag_path.qpic", true);
	// two_qubit_gate_cancellation(qc_path);
	// write_dot(qc_path, "qc_dag_path_opt.dot");

	// netlist<mct_gate> netlist;
	// kitty::dynamic_truth_table tt(3);
	// kitty::create_from_hex_string(tt, "E8");
	// esop_based_synthesis(netlist, tt);
	// write_quil(netlist, "test.quil");
	return EXIT_SUCCESS;
}
