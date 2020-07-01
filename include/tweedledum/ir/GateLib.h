/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <array>
#include <complex>
#include <kitty/kitty.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/networks/xag.hpp>
#include <string_view>

namespace tweedledum::GateLib {

class X {
	using Matrix = std::array<std::complex<double>, 4>;

public:
	static std::string_view kind()
	{
		return "x";
	}

	Matrix matrix() const
	{
		return {{{0., 0.}, {1., 0.}, {1., 0.}, {0., 0.}}};
	}
};

class H {
	using Matrix = std::array<std::complex<double>, 4>;
	constexpr static double _1_sqrt2 = 0.707106781186547524401;

public:
	static std::string_view kind()
	{
		return "h";
	}

	Matrix matrix() const
	{
		return {{{_1_sqrt2, 0.}, {_1_sqrt2, 0.}, {_1_sqrt2, 0.}, {-_1_sqrt2, 0.}}};
	}
};

class R1 {
	using Matrix = std::array<std::complex<double>, 4>;

public:
	static std::string_view kind()
	{
		return "r1";
	}

	R1(double angle)
	    : angle_(angle)
	{}

	Matrix matrix() const
	{
		return {{{1., 0.}, {0., 0.}, {0., 0.}, std::exp(std::complex<double>(0., angle_))}};
	}

private:
	double angle_;
};

class Parity {
public:
	static std::string_view kind()
	{
		return "parity";
	}
};

class TruthTable {
public:
	static std::string_view kind()
	{
		return "truth_table";
	}

	TruthTable(std::string_view name, kitty::dynamic_truth_table const& tt)
	    : name_(name), truth_table_(tt)
	{}

	std::string_view name() const
	{
		return name_;
	}

	kitty::dynamic_truth_table const& truth_table() const
	{
		return truth_table_;
	}

private:
	std::string const name_;
	kitty::dynamic_truth_table const truth_table_;
};

class AIGNetwork {
public:
	static std::string_view kind()
	{
		return "aig_network";
	}

	AIGNetwork(std::string_view name, mockturtle::aig_network const& aig)
	    : name_(name), aig_(aig)
	{}

	std::string_view name() const
	{
		return name_;
	}

	mockturtle::aig_network const& aig() const
	{
		return aig_;
	}

private:
	std::string const name_;
	mockturtle::aig_network const aig_;
};

class XAGNetwork {
public:
	static std::string_view kind()
	{
		return "xag_network";
	}

	XAGNetwork(std::string_view name, mockturtle::xag_network const& xag)
	    : name_(name), xag_(xag)
	{}

	std::string_view name() const
	{
		return name_;
	}

	mockturtle::xag_network const& aig() const
	{
		return xag_;
	}

private:
	std::string const name_;
	mockturtle::xag_network const xag_;
};

} // namespace tweedledum::GateLib
