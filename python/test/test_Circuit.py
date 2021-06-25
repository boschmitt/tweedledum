# ------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
# ------------------------------------------------------------------------------
from tweedledum.ir import Circuit


def test_wires():
    """Adding Cbits and Qubits to a circuit."""
    circuit = Circuit()
    q0 = circuit.create_qubit()
    c0 = circuit.create_cbit()
    c1 = circuit.create_cbit()
    q1 = circuit.create_qubit()

    assert circuit.num_cbits() == 2
    assert circuit.num_qubits() == 2
    assert circuit.num_wires() == 4
    assert q0.uid() == c0.uid()
    assert q1.uid() == c1.uid()
    assert q0.uid() != c1.uid()
