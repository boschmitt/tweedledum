/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

namespace tweedledum {

struct Wire {
	enum Kind { classical, quantum };

	uint32_t const uid;
	std::string const name;
	Kind const kind;

	Wire(uint32_t uid, std::string_view name, Kind kind)
	    : uid(uid), name(name), kind(kind)
	{}
};

class WireRef {
public:
	// Copy constructor
	WireRef(WireRef const& other) : data_(other.data_) {}

	// Copy assignment
	WireRef& operator=(WireRef const& other)
	{
		data_ = other.data_;
		return *this;
	}

	// Return the sentinel value
	// TODO: possibly use std::optional instead
	static WireRef invalid()
	{
		return WireRef();
	}

	uint32_t uid() const
	{
		return uid_;
	}

	Wire::Kind kind() const
	{
		return kind_;
	}

	enum Polarity { positive, negative };

	Polarity polarity() const
	{
		return static_cast<Polarity>(polarity_);
	}

	void complement()
	{
		polarity_ ^= 1u;
	}

	bool is_complemented()
	{
		return polarity_;
	}

	WireRef operator!() const
	{
		WireRef complemented(*this);
		complemented.polarity_ ^= 1u;
		return complemented;
	}

	bool operator==(WireRef other) const
	{
		return data_ == other.data_;
	}

	bool operator!=(WireRef other) const
	{
		return data_ != other.data_;
	}

	operator uint32_t() const
	{
		return uid_;
	}

	friend void to_json(nlohmann::json& j, WireRef const& wire_ref);

protected:
	static WireRef qubit(uint32_t uid)
	{
		return WireRef(uid, Wire::Kind::quantum);
	}

	static WireRef cbit(uint32_t uid)
	{
		return WireRef(uid, Wire::Kind::classical);
	}

	WireRef(uint32_t uid, Wire::Kind k, Polarity p = Polarity::positive)
	    : uid_(uid), kind_(k), polarity_(p)
	{}

	union {
		uint32_t data_;
		struct {
			uint32_t const uid_ : 30;
			Wire::Kind const kind_ : 1;
			uint32_t polarity_ : 1;
		};
	};

	friend class WireStorage;

private:
	WireRef() : data_(std::numeric_limits<uint32_t>::max()) {}
};

} // namespace tweedledum
