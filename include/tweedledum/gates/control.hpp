/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*------------------------------------------------------------------------------------------------*/
#pragma once

namespace tweedledum {

class control_t {
public:
  control_t() = default;

	control_t(uint32_t index)
	    : literal_(index << 1)
	{}
	control_t(uint32_t index, bool complemented)
	    : literal_((index << 1) | (complemented ? 1 : 0))
	{}

	inline auto index() const
	{
		return literal_ >> 1;
	}

	inline auto is_complemented() const
	{
		return (literal_ & 1) == 1;
	}

  inline auto literal() const
  {
    return literal_;
  }

  control_t operator!() const
  {
    control_t c_;
    c_.literal_ = literal_ ^ 1;
    return c_;
  }

  inline void complement()
  {
    literal_ ^= 1;
  }

  inline operator uint32_t() const
  {
    return literal_ >> 1;
  }

  inline bool operator<(control_t const& other) const
  {
    return literal_ < other.literal_;
  }

  inline bool operator==(control_t const& other) const
  {
    return literal_ == other.literal_;
  }

  inline bool operator!=(control_t const& other) const
  {
    return literal_ != other.literal_;
  }

private:
	uint32_t literal_{0};
};

} // namespace tweedledum
