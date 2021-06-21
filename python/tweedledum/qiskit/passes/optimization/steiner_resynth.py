# -------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
# -------------------------------------------------------------------------------
from typing import Union

from qiskit.dagcircuit import DAGCircuit
from qiskit.transpiler.basepasses import TransformationPass
from qiskit.transpiler.coupling import CouplingMap
from qiskit.transpiler.exceptions import TranspilerError

from tweedledum.qiskit import from_qiskit, to_qiskit
from tweedledum.ir import Circuit
from tweedledum.passes import steiner_resynth
from tweedledum.target import Device


class SteinerResynth(TransformationPass):
    """TODO"""

    def __init__(self, cmap_or_device: Union[list, CouplingMap, Device]):
        """SteinerResynth initializer.

        Args:
            cmap_or_device (Union[list, CouplingMap, Device]): Directed graph
                represented as a list of edges, or a qiskit coupling map, or a
                tweedledum Device.
        """
        super().__init__()
        if isinstance(cmap_or_device, list):
            self.device = Device.from_edge_list(cmap_or_device)
        elif isinstance(cmap_or_device, CouplingMap):
            self.device = Device.from_edge_list(cmap_or_device.get_edges())
        elif isinstance(cmap_or_device, Device):
            self.device = cmap_or_device
        else:
            raise TranspilerError("Must pass a qiskit CouplingMap or tweedledum Device")

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
