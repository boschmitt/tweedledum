/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "Wire.h"

#include <cstdint>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

namespace tweedledum {

class Operand : public Wire {
public:
	enum Polarity { positive, negative };

	Operand(Wire wire, Polarity polarity = Polarity::positive)
	    : Wire(wire.uid(), wire.kind(), polarity)
	{}

	Polarity polarity() const
	{
		return static_cast<Polarity>(subclass_data_);
	}

	Operand operator!() const
	{
		Operand complemented(*this);
		complemented.subclass_data_ ^= 1u;
		return complemented;
	}

	friend void to_json(nlohmann::json& j, Operand const& opnd);
};

} // namespace tweedledum
