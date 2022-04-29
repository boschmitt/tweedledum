/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/

#include "tweedledum/Dialect/Quirky/IR/QuirkyDialect.h"
#include "tweedledum/Dialect/Quirky/IR/QuirkyOps.h"
#include "mlir/IR/DialectImplementation.h"

using namespace tweedledum;
using namespace quirky;

using mlir::DialectAsmParser;
using mlir::OpaqueType;
using mlir::StringAttr;
using mlir::Type;

/// Parse a type registered with this dialect.
Type QuirkyDialect::parseType(DialectAsmParser &parser) const {
    StringAttr ns = StringAttr::get(getContext(), getNamespace());
    return OpaqueType::get(ns, parser.getFullSymbolSpec());
}

#include "tweedledum/Dialect/Quirky/IR/QuirkyDialect.cpp.inc"

void QuirkyDialect::initialize()
{
    addOperations<
#define GET_OP_LIST
#include "tweedledum/Dialect/Quirky/IR/Quirky.cpp.inc"
    >();
}
