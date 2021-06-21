# -------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
# -------------------------------------------------------------------------------
from tweedledum.ir import Circuit, Qubit, Cbit, rotation_angle
from tweedledum import operators as Op

from qiskit.circuit import (
    QuantumCircuit,
    QuantumRegister,
    ClassicalRegister,
    Gate,
    ControlledGate,
    Measure,
)
from qiskit.dagcircuit.dagcircuit import DAGCircuit
from qiskit.circuit.library.standard_gates import (
    HGate,
    IGate,
    PhaseGate,
    RXGate,
    RYGate,
    RZGate,
    RXXGate,
    RYYGate,
    RZZGate,
    SGate,
    SdgGate,
    SXGate,
    SXdgGate,
    SwapGate,
    TGate,
    TdgGate,
    XGate,
    YGate,
    ZGate,
)

_TO_TWEEDLEDUM_OP = {
    "h": Op.H,
    "p": Op.P,
    "rx": Op.Rx,
    "ry": Op.Ry,
    "rz": Op.Rz,
    "rxx": Op.Rxx,
    "ryy": Op.Ryy,
    "rzz": Op.Rzz,
    "s": Op.S,
    "sdg": Op.Sdg,
    "sx": Op.Sx,
    "sxdg": Op.Sxdg,
    "swap": Op.Swap,
    "t": Op.T,
    "tdg": Op.Tdg,
    "x": Op.X,
    "y": Op.Y,
    "z": Op.Z,
    "measure": Op.Measure,
}

_TO_QISKIT_GATE = {
    "std.h": HGate,
    "std.p": PhaseGate,
    "std.m": Measure,
    "std.rx": RXGate,
    "std.ry": RYGate,
    "std.rz": RZGate,
    "std.s": SGate,
    "std.sdg": SdgGate,
    "std.sx": SXGate,
    "std.sxdg": SXdgGate,
    "std.swap": SwapGate,
    "std.t": TGate,
    "std.tdg": TdgGate,
    "std.x": XGate,
    "std.y": YGate,
    "std.z": ZGate,
    "ising.rxx": RXXGate,
    "ising.ryy": RYYGate,
    "ising.rzz": RZZGate,
}


def _convert_qiskit_operator(gate):
    op = _TO_TWEEDLEDUM_OP.get(gate.name) or _TO_TWEEDLEDUM_OP.get(gate.base_gate.name)
    ctrl_state = ""
    if isinstance(gate, ControlledGate):
        ctrl_state = "{:0{}b}".format(gate.ctrl_state, gate.num_ctrl_qubits)[::-1]
    if op == None:
        return gate, ctrl_state
    if isinstance(gate.params, list) and len(gate.params) > 0:
        if len(gate.params) == 1:
            return op(gate.params[0]), ctrl_state
        else:
            return gate, ctrl_state
    return op(), ctrl_state


def _from_qc(qiskit_qc):
    circuit = Circuit()
    qubits = [circuit.create_qubit() for i in range(len(qiskit_qc.qubits))]
    qubits_map = dict(zip(qiskit_qc.qubits, qubits))
    cbits = [circuit.create_cbit() for i in range(len(qiskit_qc.clbits))]
    cbits_map = dict(zip(qiskit_qc.clbits, cbits))

    for instruction, qargs, cargs in qiskit_qc.data:
        op, ctrl_state = _convert_qiskit_operator(instruction)
        qs = [qubits_map.get(qubit) for qubit in qargs]
        cs = [cbits_map.get(cbit) for cbit in cargs]
        for i, polarity in enumerate(ctrl_state):
            if polarity == "0":
                qs[i] = ~qs[i]
        circuit.apply_operator(op, qs, cs)
    return circuit


def _from_dag(qiskit_dag):
    circuit = Circuit()
    qubits = [circuit.create_qubit() for i in range(len(qiskit_dag.qubits))]
    qubits_map = dict(zip(qiskit_dag.qubits, qubits))
    cbits = [circuit.create_cbit() for i in range(len(qiskit_dag.clbits))]
    cbits_map = dict(zip(qiskit_dag.clbits, cbits))

    for node in qiskit_dag.op_nodes():
        op, ctrl_state = _convert_qiskit_operator(node.op)
        qs = [qubits_map.get(qubit) for qubit in node.qargs]
        cs = [cbits_map.get(cbit) for cbit in node.cargs]
        for i, polarity in enumerate(ctrl_state):
            if polarity == "0":
                qs[i] = ~qs[i]
        circuit.apply_operator(op, qs, cs)
    return circuit


def _convert_tweedledum_op(op):
    base_gate = _TO_QISKIT_GATE.get(op.kind())
    if base_gate == None:
        if op.kind() == "py_op":
            return op.py_op()
        else:
            raise RuntimeError(f"Unrecognized operator: {op.kind()}")

    if op.kind() in [
        "std.p",
        "std.rx",
        "std.ry",
        "std.rz",
        "ising.rxx",
        "ising.ryy",
        "ising.rzz",
    ]:
        angle = rotation_angle(op)
        gate = base_gate(angle)
    else:
        gate = base_gate()

    # TODO: need to deal with cbits too!
    if op.num_controls() > 0:
        qubits = op.qubits()
        ctrl_state = ""
        for qubit in qubits[: op.num_controls()]:
            ctrl_state += "{}".format(int(qubit.polarity() == Qubit.Polarity.positive))
        return gate.control(len(ctrl_state), ctrl_state=ctrl_state[::-1])
    return gate


def _to_qc(circuit):
    qiskit_qc = QuantumCircuit()
    # Qiskit is weird, it doesn't allow passing 0 as size for register.
    if circuit.num_cbits():
        qiskit_qc.add_register(circuit.num_qubits(), circuit.num_cbits())
    else:
        qiskit_qc.add_register(circuit.num_qubits())

    for instruction in circuit:
        gate = _convert_tweedledum_op(instruction)
        qubits = [qubit.uid() for qubit in instruction.qubits()]
        cbits = [cbit.uid() for cbit in instruction.cbits()]
        qiskit_qc.append(gate, qubits, cbits)
    return qiskit_qc


def _to_dag(circuit):
    qiskit_dag = DAGCircuit()
    qreg = QuantumRegister(circuit.num_qubits())
    qiskit_dag.add_qreg(qreg)
    if circuit.num_cbits():
        creg = ClassicalRegister(circuit.num_cbits())
        qiskit_dag.add_creg(creg)
    for instruction in circuit:
        gate = _convert_tweedledum_op(instruction)
        qubits = [qreg[qubit.uid()] for qubit in instruction.qubits()]
        cbits = [creg[cbit.uid()] for cbit in instruction.cbits()]
        qiskit_dag.apply_operation_back(gate, qubits, cbits)
    return qiskit_dag


def from_qiskit(circuit):
    if isinstance(circuit, QuantumCircuit):
        return _from_qc(circuit)
    elif isinstance(circuit, DAGCircuit):
        return _from_dag(circuit)
    raise TypeError("Circuit must be either a QuantumCircuit or DAGCircuit")


def to_qiskit(circuit, circuit_type="dag"):
    if circuit_type == "dag":
        return _to_dag(circuit)
    elif circuit_type == "gatelist":
        return _to_qc(circuit)
    raise TypeError(f"Unrecognized circuit type {circuit_type}")
