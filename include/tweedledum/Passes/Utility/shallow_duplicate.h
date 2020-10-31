/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../IR/Wire.h"

#include <algorithm>
#include <string>

namespace tweedledum {

/*! \brief Creates a new circuit with same wires as the original.
 *
 * \param[in] original A quantum circuit (__will not be modified__).
 * \returns a __new__ circuit without operators.
 */
inline Circuit shallow_duplicate(Circuit const& original)
{
    Circuit duplicate;
    std::for_each(original.begin_wire(), original.end_wire(),
    [&](Wire const& wire) {
        switch (wire.kind) {
        case Wire::Kind::classical:
            duplicate.create_cbit(wire.name);
            break;

        case Wire::Kind::quantum:
            duplicate.create_qubit(wire.name);
            break;
        }
    });
    return duplicate;
}

} // namespace tweedledum
