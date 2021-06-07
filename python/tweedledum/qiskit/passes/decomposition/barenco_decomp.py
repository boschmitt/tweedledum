# -------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
# -------------------------------------------------------------------------------
from typing import Union

from qiskit.transpiler.basepasses import TransformationPass
from qiskit.dagcircuit import DAGCircuit

from tweedledum.qiskit import from_qiskit, to_qiskit
from tweedledum.ir import Circuit
from tweedledum.passes import barenco_decomp


class BarencoDecomp(TransformationPass):
    """TODO"""

    def __init__(self):
        """BarencoDecomp initializer."""
        super().__init__()

    def run(self, dag: Union[DAGCircuit, Circuit]) -> Union[DAGCircuit, Circuit]:
        """Run the BarencoDecomp pass on a circuit.

        Args:
            dag (Union[DAGCircuit, Circuit]): A circuit to map.

        Returns:
            Union[DAGCircuit, Circuit]: An optimized circuit.
        """
        if isinstance(dag, DAGCircuit):
            circuit = from_qiskit(dag)
        else:
            circuit = dag

        opt_circuit = barenco_decomp(circuit)

        if isinstance(dag, DAGCircuit):
            return to_qiskit(opt_circuit)
        return opt_circuit
