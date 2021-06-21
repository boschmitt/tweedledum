# -------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
# -------------------------------------------------------------------------------
from typing import Union

from qiskit.dagcircuit import DAGCircuit
from qiskit.transpiler.coupling import CouplingMap
from qiskit.transpiler.basepasses import TransformationPass

from tweedledum.qiskit import from_qiskit, to_qiskit
from tweedledum.ir import Circuit
from tweedledum.passes import steiner_resynth
from tweedledum.target import Device


class SteinerResynth(TransformationPass):
    """TODO"""

    def __init__(self, coupling_map: CouplingMap):
        """SteinerResynth initializer.

        Args:
            coupling_map (CouplingMap): Directed graph represented a coupling map.
        """
        super().__init__()
        self.device = Device.from_edge_list(coupling_map.get_edges())

    def run(self, dag: Union[DAGCircuit, Circuit]) -> Union[DAGCircuit, Circuit]:
        """Run the LinearResynth pass on a circuit.

        Args:
            dag (Union[DAGCircuit, Circuit]): A circuit to map.

        Returns:
            Union[DAGCircuit, Circuit]: An optimized circuit.
        """
        if isinstance(dag, DAGCircuit):
            circuit = from_qiskit(dag)
        else:
            circuit = dag

        opt_circuit = steiner_resynth(self.device, circuit)

        if isinstance(dag, DAGCircuit):
            return to_qiskit(opt_circuit)
        return opt_circuit
