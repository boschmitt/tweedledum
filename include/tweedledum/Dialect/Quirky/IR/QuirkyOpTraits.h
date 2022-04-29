/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "mlir/IR/OpDefinition.h"

namespace tweedledum {
namespace quirky {

template<typename ConcreteType>
class HermitianOp
    : public mlir::OpTrait::TraitBase<ConcreteType, HermitianOp> {};

} // namespace quirky
} // namespace tweedledum
