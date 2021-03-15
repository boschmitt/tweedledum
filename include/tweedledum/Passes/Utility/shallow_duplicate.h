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
    original.foreach_cbit([&](std::string_view name) {
        duplicate.create_cbit(name);
    });
    original.foreach_qubit([&](std::string_view name) {
        duplicate.create_qubit(name);
    });
    return duplicate;
}

} // namespace tweedledum
