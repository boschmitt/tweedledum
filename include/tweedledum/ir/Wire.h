/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <cstdint>
#include <fmt/format.h>

namespace tweedledum {

class Wire {
public:
	enum Kind { classical, quantum };

	// Return the sentinel value
	// TODO: possibly use std::optional instead
	static Wire invalid()
	{
		return Wire();
	}

	static Wire qubit(uint32_t uid)
	{
		return Wire(uid, Kind::quantum);
	}

	static Wire cbit(uint32_t uid)
	{
		return Wire(uid, Kind::classical);
	}

	uint32_t uid() const
	{
		return uid_;
	}

	Kind kind() const
	{
		return kind_;
	}

protected:
	Wire(uint32_t uid, Kind kind, uint32_t subclass_data = 0)
	    : uid_(uid), kind_(kind), subclass_data_(subclass_data)
	{}

	union {
		uint32_t data_;
		struct {
			uint32_t const uid_ : 30;
			Kind const kind_ : 1;
			uint32_t subclass_data_ : 1;
		};
	};

private:
	Wire() : data_(std::numeric_limits<uint32_t>::max()) {}
};

} // namespace tweedledum
