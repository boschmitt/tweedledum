/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../utils/angle.hpp"

#include <cassert>

namespace tweedledum {

enum class gate_ids : uint8_t {
#define GATE(X, Y, Z, V) X,
#include "gate_lib.def"
	num_defined_ops,
};
static_assert(static_cast<uint32_t>(gate_ids::num_defined_ops) <= 0xFF);

enum class rot_axis : uint8_t {
	na, // not applicable
	x,
	y,
	z,
	xy,
	xyz,
};

#pragma region Internal details
namespace detail {

struct gate_table_entry {
	gate_ids adjoint;
	rot_axis axis;
	char const* symbol;
};

constexpr gate_table_entry gate_info[] = {
#define GATE(X, Y, Z, V) {gate_ids::Y, rot_axis::Z, V},
#include "gate_lib.def"
};

} // namespace detail
#pragma endregion

// I do not want to have different classes for the different gate categories (Meta,
// non-parameterisable, and parameterisable), so I will define one that can represent all of them.
//
// TODO: Should ``input`` be considered one-qubit gates ?

/*! \brief Simple class to hold information about the gate */
class gate {
	// Helper function to set the info bits!
	constexpr uint32_t resolve_info_bits(gate_ids id) const
	{
		uint32_t info_bits = 0u;
		switch (id) {
		// Meta gates
		case gate_ids::input:
			info_bits |= is_meta_;
			break;

		case gate_ids::measure_x:
		case gate_ids::measure_y:
		case gate_ids::measure_z:
			info_bits |= is_measurement_;
			break;

		// Non-parameterisable gates
		case gate_ids::i:
		case gate_ids::h:
		case gate_ids::x:
		case gate_ids::y:
			info_bits |= is_one_qubit_;
			break;

		case gate_ids::z:
		case gate_ids::s:
		case gate_ids::sdg:
		case gate_ids::t:
		case gate_ids::tdg:
			info_bits |= is_r1_;
			info_bits |= is_one_qubit_;
			break;

		case gate_ids::cx:
			info_bits |= is_two_qubit_;
			break;

		case gate_ids::cy:
			info_bits |= is_two_qubit_;
			break;

		case gate_ids::cz:
			info_bits |= is_two_qubit_;
		case gate_ids::ncz:
			info_bits |= is_r1_;
			break;

		case gate_ids::swap:
			info_bits |= is_two_qubit_;
			break;

		// Parameterisable gates
		case gate_ids::u3:
			info_bits |= is_parameterisable_;
			info_bits |= is_one_qubit_;
			break;

		case gate_ids::r1:
			info_bits |= is_parameterisable_;
			info_bits |= is_r1_;
			info_bits |= is_one_qubit_;
			break;

		case gate_ids::rx:
			info_bits |= is_parameterisable_;
			info_bits |= is_one_qubit_;
			break;

		case gate_ids::ry:
			info_bits |= is_parameterisable_;
			info_bits |= is_one_qubit_;
			break;

		case gate_ids::rz:
			info_bits |= is_parameterisable_;
			info_bits |= is_one_qubit_;
			info_bits |= is_rz_;
			break;

		case gate_ids::crx:
			info_bits |= is_parameterisable_;
			info_bits |= is_two_qubit_;
			break;

		case gate_ids::cry:
			info_bits |= is_parameterisable_;
			info_bits |= is_two_qubit_;
			break;

		case gate_ids::crz:
			info_bits |= is_parameterisable_;
			info_bits |= is_two_qubit_;
			info_bits |= is_rz_;
			break;

		case gate_ids::ncrx:
			info_bits |= is_parameterisable_;
			break;

		case gate_ids::ncry:
			info_bits |= is_parameterisable_;
			break;

		case gate_ids::ncrz:
			info_bits |= is_parameterisable_;
			info_bits |= is_rz_;
			break;

		default:
			break;
		}
		return info_bits;
	}

#pragma region Constructors
public:
	explicit gate(gate_ids id, angle const& theta, angle const& phi, angle const& lambda)
	    : id_(id)
	    , id_adjoint_(detail::gate_info[static_cast<uint32_t>(id)].adjoint)
	    , axis_(detail::gate_info[static_cast<uint32_t>(id)].axis)
	    , info_bits_(resolve_info_bits(id))
	    , theta_(theta)
	    , phi_(phi)
	    , lambda_(lambda)
	{
		assert((info_bits_ & is_parameterisable_) && "You should NOT be using this!");
		(void) pad_;
	}

	// I want to differentiate between the constexpr constructor and the non-constexpr, but
	// they take the same arguments.  So to change the signature I inverted the order of the
	// arguments ^-^
	//
	// Users should not use this constructor!
	constexpr gate(angle const& theta, angle const& phi, angle const& lambda, gate_ids id)
	    : id_(id)
	    , id_adjoint_(detail::gate_info[static_cast<uint32_t>(id)].adjoint)
	    , axis_(detail::gate_info[static_cast<uint32_t>(id)].axis)
	    , info_bits_(resolve_info_bits(id))
	    , theta_(theta)
	    , phi_(phi)
	    , lambda_(lambda)
	{}
#pragma endregion

#pragma region Properties
	constexpr gate_ids id() const
	{
		return id_;
	}

	constexpr bool is(gate_ids gate_ids) const
	{
		return id() == gate_ids;
	}

	constexpr bool is_adjoint(gate const& other) const
	{
		// For the non-parameterisable gates, this test is enough:
		if (this->id_adjoint_ != other.id_) {
			return false;
		}
		// For the parameterisables gates, I need to make sure that the angles cancel out:
		switch (id()) {
		case gate_ids::r1:
		case gate_ids::rx:
		case gate_ids::ry:
		case gate_ids::rz:
		case gate_ids::crx:
		case gate_ids::cry:
		case gate_ids::crz:
		case gate_ids::ncrx:
		case gate_ids::ncry:
		case gate_ids::ncrz:
			if (rotation_angle() + other.rotation_angle() != sym_angle::zero) {
				return false;
			}
			return true;

		case gate_ids::u3:
			if ((theta_ + other.theta_) != sym_angle::zero) {
				return false;
			} else if ((phi_ + other.phi_) != sym_angle::zero) {
				return false;
			} else if ((lambda_ + other.lambda_) != sym_angle::zero) {
				return false;
			}
			return true;

		default:
			break;
		}
		return true;
	}

	constexpr bool is_meta() const
	{
		return info_bits_ & is_meta_;
	}

	constexpr bool is_one_qubit() const
	{
		return info_bits_ & is_one_qubit_;
	}

	constexpr bool is_two_qubit() const
	{
		return info_bits_ & is_two_qubit_;
	}

	constexpr bool is_r1() const
	{
		return info_bits_ & is_r1_;
	}

	constexpr bool is_measurement() const
	{
		return info_bits_ & is_measurement_;
	}

	constexpr rot_axis axis() const
	{
		return axis_;
	}
#pragma endregion

#pragma region Parameters
	/* !brief Return the rotation angle */
	constexpr angle rotation_angle() const
	{
		assert(!is_meta() && !is_measurement() && !is(gate_ids::swap) && !is(gate_ids::u3));
		if (is_r1()) {
			return lambda_;
		}
		return theta_;
	}

	constexpr angle theta() const
	{
		assert(is(gate_ids::u3));
		return theta_;
	}

	constexpr angle phi() const
	{
		assert(is(gate_ids::u3));
		return phi_;
	}

	constexpr angle lambda() const
	{
		assert(is(gate_ids::u3));
		return lambda_;
	}
#pragma endregion

#pragma region Overloads
	constexpr bool operator!=(gate const& other) const
	{
		if (id_ != other.id_) {
			return true;
		}
		if (theta_ != other.theta_) {
			return true;
		}
		if (phi_ != other.phi_) {
			return true;
		}
		return lambda_ != other.lambda_;
	}
#pragma endregion

private:
	// A lean ``gate`` object would take only the identifier and three angle parameters.
	// However, the alignment would be off, so the compiler will align by padding it (add bytes)
	// Well, rather than letting this padding happen, which effectively waste space, let's add
	// it ourselves, and put this space to good use:
	//     * I will have the gate to carry both its ``id`` and it's adjoint gate ``id``. This
	//       can simplyfy testing whether two gates are adjoint or not.
	//
	//     * I wil also add an hand implemented bit field, `info_bits_`, to store some useful
	//       information about the gate. The names are pretty much self-explanatory:
	enum : uint32_t {
		is_meta_ = (1u << 0),
		is_parameterisable_ = (1u << 1),
		is_one_qubit_ = (1u << 2),
		is_two_qubit_ = (1u << 3),
		is_r1_ = (1u << 4),
		is_rz_ = (1u << 5),
		is_measurement_ = (1u << 6)
	};
	//
	// FIXME: I will leave this tag here because I see that on the long term this decison will
	//        come to bite me ^-^
	const uint8_t pad_ = 0u;
	gate_ids id_;
	gate_ids id_adjoint_;
	rot_axis axis_;
	uint32_t info_bits_;
	angle theta_;
	angle phi_;
	angle lambda_;
};

// Gate library
namespace gate_lib {

// Meta gates
constexpr gate undefined(sym_angle::zero, sym_angle::zero, sym_angle::zero, gate_ids::undefined);
constexpr gate opaque(sym_angle::zero, sym_angle::zero, sym_angle::zero, gate_ids::opaque);
constexpr gate input(sym_angle::zero, sym_angle::zero, sym_angle::zero, gate_ids::input);

// Measurement gates
constexpr gate measure_x(sym_angle::zero, sym_angle::zero, sym_angle::zero, gate_ids::measure_x);
constexpr gate measure_y(sym_angle::zero, sym_angle::zero, sym_angle::zero, gate_ids::measure_y);
constexpr gate measure_z(sym_angle::zero, sym_angle::zero, sym_angle::zero, gate_ids::measure_z);

// One-qubit gates
constexpr gate i(sym_angle::zero, sym_angle::zero, sym_angle::zero, gate_ids::i);
constexpr gate h(sym_angle::pi_half, sym_angle::zero, sym_angle::pi, gate_ids::h);
constexpr gate x(sym_angle::pi, sym_angle::zero, sym_angle::pi, gate_ids::x);
constexpr gate y(sym_angle::pi, sym_angle::pi_half, sym_angle::pi_half, gate_ids::y);
constexpr gate z(sym_angle::zero, sym_angle::zero, sym_angle::pi, gate_ids::z);
constexpr gate s(sym_angle::zero, sym_angle::zero, sym_angle::pi_half, gate_ids::s);
constexpr gate sdg(sym_angle::zero, sym_angle::zero, -sym_angle::pi_half, gate_ids::sdg);
constexpr gate t(sym_angle::zero, sym_angle::zero, sym_angle::pi_quarter, gate_ids::t);
constexpr gate tdg(sym_angle::zero, sym_angle::zero, -sym_angle::pi_quarter, gate_ids::tdg);

// Two-qubit unitary gates
constexpr gate cx(sym_angle::pi, sym_angle::zero, sym_angle::pi, gate_ids::cx);
constexpr gate cy(sym_angle::pi, sym_angle::pi_half, sym_angle::pi_half, gate_ids::cy);
constexpr gate cz(sym_angle::zero, sym_angle::zero, sym_angle::pi, gate_ids::cz);
constexpr gate swap(sym_angle::zero, sym_angle::zero, sym_angle::zero, gate_ids::swap);

// N-qubit unitary gates
constexpr gate ncx(sym_angle::pi, sym_angle::zero, sym_angle::pi, gate_ids::ncx);
constexpr gate ncy(sym_angle::pi, sym_angle::pi_half, sym_angle::pi_half, gate_ids::ncy);
constexpr gate ncz(sym_angle::zero, sym_angle::zero, sym_angle::pi, gate_ids::ncz);

// Functions to create parameterisable gates
inline gate u3(angle theta, angle phi, angle lambda)
{
	return gate(gate_ids::u3, theta, phi, lambda);
}

inline gate r1(angle const& lambda)
{
	return gate(gate_ids::r1, sym_angle::zero, sym_angle::zero, lambda);
}

inline gate identified_r1(angle const& lambda)
{
	if (!lambda.is_numerically_defined()) {
		if (lambda == sym_angle::pi_quarter) {
			return t;
		} else if (lambda == -sym_angle::pi_quarter) {
			return tdg;
		} else if (lambda == sym_angle::pi_half) {
			return s;
		} else if (lambda == -sym_angle::pi_half) {
			return sdg;
		} else if (lambda == sym_angle::pi || lambda == -sym_angle::pi) {
			return z;
		}
	}
	return gate(gate_ids::r1, sym_angle::zero, sym_angle::zero, lambda);
}

inline gate rx(angle const& theta)
{
	return gate(gate_ids::rx, theta, -sym_angle::pi_half, sym_angle::pi_half);
}

inline gate ry(angle const& theta)
{
	return gate(gate_ids::ry, theta, sym_angle::zero, sym_angle::zero);
}

inline gate rz(angle const& theta)
{
	return gate(gate_ids::rz, theta, sym_angle::zero, sym_angle::zero);
}

inline gate crx(angle const& theta)
{
	return gate(gate_ids::crx, theta, -sym_angle::pi_half, sym_angle::pi_half);
}

inline gate cry(angle const& theta)
{
	return gate(gate_ids::cry, theta, sym_angle::zero, sym_angle::zero);
}

inline gate crz(angle const& theta)
{
	return gate(gate_ids::crz, theta, sym_angle::zero, sym_angle::zero);
}

inline gate ncrx(angle const& theta)
{
	return gate(gate_ids::ncrx, theta, -sym_angle::pi_half, sym_angle::pi_half);
}

inline gate ncry(angle const& theta)
{
	return gate(gate_ids::ncry, theta, sym_angle::zero, sym_angle::zero);
}

inline gate ncrz(angle const& theta)
{
	return gate(gate_ids::ncrz, theta, sym_angle::zero, sym_angle::zero);
}

} // namespace gate_lib

namespace gate_set {
namespace detail {

template<typename... Gates>
constexpr uint64_t unpack_gates(gate_ids g)
{
	return (1ull << static_cast<uint64_t>(g));
}

template<typename... Gates>
constexpr uint64_t unpack_gates(gate_ids g0, Gates... gn)
{
	return unpack_gates(g0) | unpack_gates(gn...);
}
} // namespace detail

template<typename... Gates>
constexpr uint64_t create(Gates... gn) 
{
	return detail::unpack_gates(gn...);
}

constexpr uint64_t ibm = create(gate_ids::u3, gate_ids::cx);

// This one is also called NCT: NOT, CNOT and Toffoli
constexpr uint64_t classic_rev = create(gate_ids::x, gate_ids::cx, gate_ids::ncx, gate_ids::swap);

constexpr uint64_t cnot_rz = create(gate_ids::x, gate_ids::z, gate_ids::s, gate_ids::t, gate_ids::sdg,
                                    gate_ids::tdg, gate_ids::cx, gate_ids::r1, gate_ids::rz);

constexpr uint64_t clifford_t = create(gate_ids::i, gate_ids::h, gate_ids::x, gate_ids::y,
                                       gate_ids::z, gate_ids::s, gate_ids::t, gate_ids::sdg,
                                       gate_ids::tdg, gate_ids::cx, gate_ids::cy, gate_ids::cz,
                                       gate_ids::swap);

} // namespace gate_set
} // namespace tweedledum
