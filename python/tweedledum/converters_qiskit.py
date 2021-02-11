#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
from tweedledum.ir import Circuit, WireRef
from tweedledum import operators as dum_ops

from qiskit import QuantumRegister
from qiskit.circuit import QuantumCircuit, Gate, ControlledGate
from qiskit.dagcircuit.dagcircuit import DAGCircuit
from qiskit.circuit.library.standard_gates import (HGate, IGate, PhaseGate, 
    RXGate, RYGate, RZGate, RXXGate, RYYGate, RZZGate, SGate, SdgGate, SwapGate,
    TGate, TdgGate, XGate, YGate, ZGate)

_to_tweedledum_op = {
    'h': dum_ops.H, 
    'p': dum_ops.P,
    'rx': dum_ops.Rx,
    'ry': dum_ops.Ry,
    'rz': dum_ops.Rz,
    'rxx': dum_ops.Rxx,
    'ryy': dum_ops.Ryy,
    'rzz': dum_ops.Rzz,
    's': dum_ops.S,
    'sdg': dum_ops.Sdg, 
    'swap': dum_ops.Swap,
    't': dum_ops.T,
    'tdg': dum_ops.Tdg, 
    'x': dum_ops.X,
    'y': dum_ops.Y,
    'z': dum_ops.Z
}

_to_qiskit_gate = {
    'std.h': HGate,
    'std.p': PhaseGate,
    'std.rx': RXGate,
    'std.ry': RYGate,
    'std.rz': RZGate,
    'std.s': SGate,
    'std.sdg': SdgGate,
    'std.swap': SwapGate,
    'std.t': TGate,
    'std.tdg': TdgGate,
    'std.x': XGate,
    'std.y': YGate,
    'std.z': ZGate,
    'ising.rxx': RXXGate,
    'ising.ryy': RYYGate,
    'ising.rzz': RZZGate
}

def _convert_qiskit_operator(gate):
    op = _to_tweedledum_op.get(gate.name) or _to_tweedledum_op.get(gate.base_gate.name)
    ctrl_state = ''
    if isinstance(gate, ControlledGate):
        ctrl_state = '{:0{}b}'.format(gate.ctrl_state, gate.num_ctrl_qubits)[::-1]
    if op == None:
        return gate, ctrl_state
    if isinstance(gate.params, list) and len(gate.params) > 0:
        if len(gate.params) == 1:
            return op(gate.params[0]), ctrl_state
        else:
            return gate, ctrl_state
    return op(), ctrl_state

def qiskit_qc_to_tweedledum(qiskit_qc):
    circuit = Circuit()
    qubits = [circuit.create_qubit() for i in range(len(qiskit_qc.qubits))]
    qubits_map = dict(zip(qiskit_qc.qubits, qubits))

    for instruction, qargs, _ in qiskit_qc.data:
        op, ctrl_state = _convert_qiskit_operator(instruction)
        qs = [qubits_map.get(qubit) for qubit in qargs]
        for i, polarity in enumerate(ctrl_state):
            if polarity == '0':
                qs[i] = ~qs[i]
        circuit.apply_operator(op, qs)
    return circuit

def qiskit_dag_to_tweedledum(qiskit_dag):
    circuit = Circuit()
    qubits = [circuit.create_qubit() for i in range(len(qiskit_dag.qubits))]
    qubits_map = dict(zip(qiskit_dag.qubits, qubits))

    for node in qiskit_dag.op_nodes():
        op, ctrl_state = _convert_qiskit_operator(node.op)
        qs = [qubits_map.get(qubit) for qubit in node.qargs]
        for i, polarity in enumerate(ctrl_state):
            if polarity == '0':
                qs[i] = ~qs[i]
        circuit.apply_operator(op, qs)
    return circuit

def _convert_tweedledum_op(op):
    base_gate = _to_qiskit_gate.get(op.kind())
    if base_gate == None:
        if op.kind() == 'py_operator':
            return op.py_op()
        else:
            raise RuntimeError(f'Unrecognized operator: {op.kind()}')

    # TODO: need to deal with cbits too!
    if op.num_controls() > 0:
        qubits = op.qubits()
        ctrl_state = ''
        for qubit in qubits[:op.num_controls()]:
            ctrl_state += '{}'.format(int(qubit.polarity() == WireRef.Polarity.positive)) 
        return base_gate().control(len(ctrl_state), ctrl_state=ctrl_state[::-1])
    return base_gate()

    # TODO:
    # elif instruction.kind() == 'std.p':
    #     return PhaseGate()
    # elif instruction.kind() == 'std.rx':
    #     return RZGate()
    # elif instruction.kind() == 'std.ry':
    #     return RZGate()
    # elif instruction.kind() == 'std.rz':
    #     return RZGate()

def tweedledum_to_qiskit_qc(circuit):
    qiskit_qc = QuantumCircuit(circuit.num_qubits())
    for instruction in circuit:
        gate = _convert_tweedledum_op(instruction)
        qubits = [qubit.uid() for qubit in instruction.qubits()]
        qiskit_qc.append(gate, qubits)
    return qiskit_qc

def tweedledum_to_qiskit_dag(circuit):
    qiskit_dag = DAGCircuit()
    qiskit_dag.add_qreg(circuit.num_qubits())
    for instruction in circuit:
        gate = _convert_tweedledum_op(instruction)
        qubits = [qubit.uid() for qubit in instruction.qubits()]
        qiskit_dag.apply_operation_back(gate, qubits)
    return qiskit_dag
