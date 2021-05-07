/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"

#include <string>

namespace tweedledum {

std::string to_string_utf8(
  Circuit const& circuit, uint32_t const max_rows = 80u);

void print(Circuit const& circuit, uint32_t const max_rows = 80u);

} // namespace tweedledum