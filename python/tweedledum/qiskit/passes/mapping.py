# -------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
# -------------------------------------------------------------------------------
"""TODO"""

from typing import Optional, Union

from qiskit.transpiler.basepasses import TransformationPass
from qiskit.transpiler.exceptions import TranspilerError
from qiskit.dagcircuit import DAGCircuit
from qiskit.transpiler.coupling import CouplingMap

from tweedledum.ir import Circuit
from tweedledum.target import Device
from tweedledum.passes import bridge_map, jit_map, sabre_map
from tweedledum.qiskit import from_qiskit, to_qiskit


class TweedledumMap(TransformationPass):
    """TODO"""

    def __init__(
        self,
        coupling_map: CouplingMap,
        method: str = "sabre",
        seed: Optional[int] = None,
        fake_run: bool = False,
    ):
        """TweedledumMap initializer.

        Args:
            coupling_map (CouplingMap): Directed graph represented a coupling map.
        """
        super().__init__()
        self.coupling_map = coupling_map
        self.method = method

    def run(self, dag: Union[DAGCircuit, Circuit]) -> Union[DAGCircuit, Circuit]:
        """Run the TweedledumMap pass on `dag`.

        Args:
            dag (DAGCircuit): DAG to map.

        Returns:
            DAGCircuit: A mapped DAG.

        Raises:
            TranspilerError: if the coupling map or the layout are not
            compatible with the DAG.
        """
        if isinstance(dag, DAGCircuit):
            self._check_qiskit(dag)
            circuit = from_qiskit(dag)
        else:
            circuit = dag

        device = Device.from_edge_list(self.coupling_map.get_edges())

        if self.method == "sabre":
            mapped_circuit, _ = sabre_map(device, circuit)
        elif self.method == "lazy":
            mapped_circuit, _ = jit_map(device, circuit)
        elif self.method == "bridge":
            mapped_circuit, _ = bridge_map(device, circuit)

        if isinstance(dag, DAGCircuit):
            return to_qiskit(mapped_circuit)
        return mapped_circuit

    def _check_qiskit(self, dag: DAGCircuit):
        if len(dag.qregs) != 1 or dag.qregs.get("q", None) is None:
            raise TranspilerError("TweedledumMap runs on physical circuits only")

        if len(dag.qubits) > len(self.coupling_map.physical_qubits):
            raise TranspilerError(
                "The layout does not match the amount of qubits in the DAG"
            )
