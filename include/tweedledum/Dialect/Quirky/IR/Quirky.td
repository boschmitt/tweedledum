/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#ifndef QUIRKY_TD
#define QUIRKY_TD

include "mlir/IR/DialectBase.td"
include "mlir/IR/BuiltinTypes.td"
include "mlir/Interfaces/InferTypeOpInterface.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

//===----------------------------------------------------------------------===//
// Dialect definition.
//===----------------------------------------------------------------------===//
def QuirkyDialect : Dialect {
    let name = "quirky";
    let summary = "Types and operations for quantum memory dialect.";
    let description = [{
        This dialect defines the `quirky` dialect, which is intended to be 
        define the core types and operations for managing quantum memory.
    }];
    let cppNamespace = "::tweedledum::quirky";
    let extraClassDeclaration = [{
        /// Type parsing and printing.
        mlir::Type parseType(mlir::DialectAsmParser &parser) const override;
    }];
}

//===----------------------------------------------------------------------===//
// Quirky Type definitions
//===----------------------------------------------------------------------===//
def QubitType : OpaqueType<"quirky", "qubit", "Opaque qubit type">;

//===----------------------------------------------------------------------===//
// Quirky OpTrait definitions
//===----------------------------------------------------------------------===//
def HermitianOp : NativeOpTrait<"HermitianOp"> {
    let cppNamespace = "::tweedledum::quirky";
}

//===----------------------------------------------------------------------===//
// Base operation definition.
//===----------------------------------------------------------------------===//
class QuirkyOp<string mnemonic, list<Trait> traits = []> :
        Op<QuirkyDialect, mnemonic, traits>;

include "tweedledum/Dialect/Quirky/IR/QuirkyOps.td"

#endif // QUIRKY_TD
