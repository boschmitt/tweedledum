#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import unittest

from tweedledum.ir import Circuit
from tweedledum.operators import H, X

class TestCircuit(unittest.TestCase):
    def test_wires(self):
        """Adding Cbits and Qubits to a circuit."""
        circuit = Circuit()
        q0 = circuit.create_qubit()
        c0 = circuit.create_cbit()
        c1 = circuit.create_cbit()
        q1 = circuit.create_qubit()

        self.assertEqual(circuit.num_cbits(), 2)
        self.assertEqual(circuit.num_qubits(), 2)
        self.assertEqual(circuit.num_wires(), 4)
        self.assertEqual(q0.uid(), c0.uid())
        self.assertEqual(q1.uid(), c1.uid())
        self.assertNotEqual(q0.uid(), c1.uid())

    def test_applying_operators(self):
        """Adding Cbits and Qubits to a circuit."""
        circuit = Circuit()
        q0 = circuit.create_qubit()
        c0 = circuit.create_cbit()
        q1 = circuit.create_qubit()

        circuit.apply_operator(H(), [q1])
        circuit.apply_operator(X(), [q1, q0])

        circuit.apply_operator(H(), [q1], [c0])
        circuit.apply_operator(X(), [q1, q0], [c0])
        # TODO: finish!
