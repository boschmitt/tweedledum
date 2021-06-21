# -------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
# -------------------------------------------------------------------------------
from typing import Optional, Union

from qiskit.dagcircuit import DAGCircuit
from qiskit.transpiler.basepasses import TransformationPass
from qiskit.transpiler.coupling import CouplingMap
from qiskit.transpiler.exceptions import TranspilerError

from tweedledum.ir import Circuit
from tweedledum.passes import bridge_map, jit_map, sabre_map
from tweedledum.qiskit import from_qiskit, to_qiskit
from tweedledum.target import Device


class Mapping(TransformationPass):
    """TODO"""

    def __init__(
        self,
        cmap_or_device: Union[list, CouplingMap, Device],
        method: str = "sabre",
        seed: Optional[int] = None,
        fake_run: bool = False,
    ):
        """Mapping initializer.

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
        self.method = method

    def run(self, dag: Union[DAGCircuit, Circuit]) -> Union[DAGCircuit, Circuit]:
        """Run the Mapping pass on `dag`.

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

        if self.method == "sabre":
            mapped_circuit, _ = sabre_map(self.device, circuit)
        elif self.method == "lazy":
            mapped_circuit, _ = jit_map(self.device, circuit)
        elif self.method == "bridge":
            mapped_circuit, _ = bridge_map(self.device, circuit)

        if isinstance(dag, DAGCircuit):
            return to_qiskit(mapped_circuit)
        return mapped_circuit

    def _check_qiskit(self, dag: DAGCircuit):
        if len(dag.qregs) != 1 or dag.qregs.get("q", None) is None:
            raise TranspilerError("Mapping runs on physical circuits only")
