#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
from tweedledum.ir import Circuit, WireRef
from tweedledum import operators as dum_ops

from qiskit import QuantumRegister
from qiskit.circuit import QuantumCircuit, Gate, ControlledGate
from qiskit.dagcircuit.dagcircuit import DAGCircuit
from qiskit.circuit.library.standard_gates import *

def _convert_qiskit_operator(op):
    ops = {'h': dum_ops.H, 'ch': dum_ops.H, 's': dum_ops.S, 'sdg': dum_ops.Sdg, 
           'swap': dum_ops.Swap, 't': dum_ops.T, 'tdg': dum_ops.Tdg, 
           'x': dum_ops.X, 'cx': dum_ops.X, 'ccx': dum_ops.X, 'mcx': dum_ops.X, 
           'y': dum_ops.Y, 'cy': dum_ops.Y, 'mcy': dum_ops.Y,
           'z': dum_ops.Z, 'cz': dum_ops.Z, 'mcz': dum_ops.Z}
    
    base_gate = ops.get(op.name)
    ctrl_state = ''
    if base_gate == None:
        raise RuntimeError(f'Unrecognized gate {op.name}')
    if isinstance(op, ControlledGate):
        ctrl_state = '{:0{}b}'.format(op.ctrl_state, op.num_ctrl_qubits)
    return base_gate(), ctrl_state[::-1]

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

def _convert_tweedledum_operator(instruction):
    ops = {'std.h': HGate, 'std.s': SGate, 'std.sdg': SdgGate, 
           'std.swap': SwapGate, 'std.t': TGate, 'std.tdg': TdgGate, 
           'std.x': XGate, 'std.y': YGate, 'std.z': ZGate}

    base_gate = ops.get(instruction.kind())
    # TODO: need to deal with cbits too!
    if instruction.num_controls() > 0:
        qubits = instruction.qubits()
        ctrl_state = ''
        for qubit in qubits[:instruction.num_controls()]:
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
        gate = _convert_tweedledum_operator(instruction)
        qubits = [qubit.uid() for qubit in instruction.qubits()]
        qiskit_qc.append(gate, qubits)
    return qiskit_qc

def tweedledum_to_qiskit_dag(circuit):
    qiskit_dag = DAGCircuit()
    qiskit_dag.add_qreg(circuit.num_qubits())
    for instruction in circuit:
        gate = _convert_tweedledum_operator(instruction)
        qubits = [qubit.uid() for qubit in instruction.qubits()]
        qiskit_dag.apply_operation_back(gate, qubits)
    return qiskit_dag
