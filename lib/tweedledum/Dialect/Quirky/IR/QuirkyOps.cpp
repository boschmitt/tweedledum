/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/

#include "tweedledum/Dialect/Quirky/IR/QuirkyOps.h"
#include "mlir/IR/OpImplementation.h"
#include "tweedledum/Dialect/Quirky/IR/QuirkyDialect.h"

using mlir::OpaqueType;
using mlir::Type;

bool isOpaqueTypeWithName(Type type, std::string dialect, std::string type_name)
{
    if (type.isa<OpaqueType>() && dialect == "quirky") {
        if (type_name == "qubit") {
            return true;
        }
    }
    return false;
}

#define GET_OP_CLASSES
#include "tweedledum/Dialect/Quirky/IR/Quirky.cpp.inc"
