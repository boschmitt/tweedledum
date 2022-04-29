/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/

def XOp : QuirkyOp<"x", [NoSideEffect, HermitianOp]> {
    let summary = "Pauli-X operation.";
    let description = [{
        The `quirky.x` operation applies Pauli-X to a the last qubit provided as 
        an argument.  The state of all other qubits is left unchanged, and they 
        are consider controls for the operation.

        This operation is Hermitian, meaning that it its self-adjoint.

        Example:

        ```mlir
        // Apply the `x` operation to %q0
        %0 = quriky.x %q0 : !quriky.qubit
        // Apply an controlled `x` operation to %0 (which is %q0)
        %1 = quirky.x %q1, %0 : !quriky.qubit, !quriky.qubit
        ```
    }];

    let arguments = (ins UnitAttr:$is_adj, Variadic<QubitType>:$qubits);
    let results = (outs QubitType:$res);

    let assemblyFormat = [{
        attr-dict (`adj` $is_adj^)? $qubits `:` type($qubits)
    }];
}
