/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <string_view>

namespace tweedledum::GateLib {

class X {
public:
	static std::string_view kind()
	{
		return "x";
	}
};

} // namespace tweedledum::GateLib
