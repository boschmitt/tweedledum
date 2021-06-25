# -------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
# -------------------------------------------------------------------------------
from .bitvec import BitVec
from .bool_function import BoolFunction

from tweedledum.ir import Circuit
from tweedledum.operators import X, H
from tweedledum.synthesis import (
    lhrs_synth,
    pprm_synth,
    pkrm_synth,
    spectrum_synth,
    xag_synth,
)


_METHOD_TO_CALLABLE = {
    "lhrs": lhrs_synth,
    "pprm": pprm_synth,
    "pkrm": pkrm_synth,
    "spectrum": spectrum_synth,
    "xag": xag_synth,
}


def bitflip_circuit(f: BoolFunction, method: str, config: dict() = {}):
    r"""Synthesizes a Boolean Function as a bitflip oracle.

    A bitflip oracle is a quantum operator `Bf` specified by a Boolean function
    `f` for which the effect on all computational basis states is given by

       Bf : |x>|y>|0>^a --> |x>|y + f(x)>|0>^a

    where `+` is the logical exclusive-or operator and `a >= 0` corresponds to
    the number of extra qubits used to store intermediate results for the
    computation of `f(x)`, the so-called ancillae qubits.

    (For clarity, this example only showed a single output function)

    Args:
        f(BoolFunction): Boolean function to be synthesized
        method: Synthesis method ('lhrs', 'pprm', 'pprm', 'spectrum', or 'xag')
        config: Dictionary with configuration parameters for tweedledum
    """
    synthesizer = _METHOD_TO_CALLABLE.get(method)
    if synthesizer is None:
        raise ValueError(f"Unrecognized synthesis method: {method}")
    if method in ["pprm", "pkrm", "spectrum"]:
        if f.num_outputs() > 1:
            raise ValueError("TT based methods only work for single output functions")
        if f.num_inputs() > 16:
            raise ValueError(
                "TT based methods only work for functions with at most 16 inputs"
            )
        return synthesizer(f.truth_table(output_bit=0), config)
    return synthesizer(f.logic_network(), config)


def phaseflip_circuit(f: BoolFunction, method: str, config: dict() = {}):
    r"""Synthesizes a Boolean Function as a phaseflip oracle.

    A phaseflip oracle is a quantum operator `Pf` specified by a Boolean
    function `f` for which the effect on all computational basis states is given
    by

        Bf : |x>|0>^a --> (-1)^{f(x)}|x>|0>^a

    Note that you can easily construct a phase oracle from a bit oracle by
    sandwiching the controlled X gate on the result qubit by a X and H gate.
    """
    synthesizer = _METHOD_TO_CALLABLE.get(method)
    if synthesizer is None:
        raise ValueError(f"Unrecognized synthesis method: {method}")
    if method in ["pprm", "pkrm", "spectrum"]:
        if f.num_outputs() > 1:
            raise ValueError("TT based methods only work for single output functions")
        if f.num_inputs() > 16:
            raise ValueError(
                "TT based methods only work for functions with at most 16 inputs"
            )
        if f"{method}_synth" not in config:
            config[f"{method}_synth"] = {"phase_esop": True}
        else:
            config[f"{method}_synth"]["phase_esop"] = True
        return synthesizer(f.truth_table(output_bit=0), config)

    circuit = Circuit()
    num_qubits = f.num_inputs() + f.num_outputs()
    qubits = [circuit.create_qubit() for _ in range(num_qubits)]
    cbits = list()
    for i in range(f.num_inputs(), num_qubits):
        circuit.apply_operator(X(), [qubits[i]])
        circuit.apply_operator(H(), [qubits[i]])
    synthesizer(circuit, qubits, cbits, f.logic_network(), config)
    for i in range(f.num_inputs(), num_qubits):
        circuit.apply_operator(H(), [qubits[i]])
        circuit.apply_operator(X(), [qubits[i]])
    return circuit
