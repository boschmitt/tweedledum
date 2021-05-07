/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"

#include <string_view>

namespace tweedledum::qasm {

Circuit parse_source_file(std::string_view path);

Circuit parse_source_buffer(std::string_view buffer);

} // namespace tweedledum::qasm
