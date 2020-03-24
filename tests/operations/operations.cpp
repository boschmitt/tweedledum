/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/gates/gate.hpp"
#include "tweedledum/networks/wire_id.hpp"
#include "tweedledum/operations/w2_op.hpp"
#include "tweedledum/operations/w3_op.hpp"
#include "tweedledum/operations/wn32_op.hpp"
#include "tweedledum/utils/angle.hpp"

#include <catch.hpp>

using namespace tweedledum;

TEMPLATE_TEST_CASE("Check correct instantiation of meta ops", "[ops][inst]", w2_op, w3_op, wn32_op)
{
	wire_id qubit(0, true);
	wire_id cbit(1, false);

	TestType q_input(gate_lib::input, qubit);
	TestType c_input(gate_lib::input, cbit);
}

template<class Op>
void check_one_wire(gate const& g, wire_id const& target)
{
	Op o(g, target);
	// Non-vector constructor
	CHECK(o.num_controls() == 0u);
	CHECK(o.num_targets() == 1u);
	CHECK(o.target() == target);

	// Vector constructor
	Op o_vec(g, std::vector<wire_id>({}), std::vector<wire_id>({target}));
	CHECK(o_vec.num_controls() == 0u);
	CHECK(o_vec.num_targets() == 1u);
	CHECK(o_vec.target() == target);

	CHECK(o == o_vec);
}

template<class Op>
void check_two_wire(gate const& g, wire_id const& control, wire_id const& target)
{
	Op o(g, control, target);
	// Non-vector constructor
	CHECK(o.num_controls() == 1u);
	CHECK(o.num_targets() == 1u);
	CHECK(o.control() == control);
	CHECK(o.target() == target);

	// Vector constructor
	Op o_vec(g, std::vector<wire_id>({control}), std::vector<wire_id>({target}));
	CHECK(o_vec.num_controls() == 1u);
	CHECK(o_vec.num_targets() == 1u);
	CHECK(o_vec.control() == control);
	CHECK(o_vec.target() == target);

	CHECK(o == o_vec);
}

TEMPLATE_TEST_CASE("Check correct instantiation of non-parameterasible 1 and 2 wires operations",
                   "[ops][inst]", w2_op, w3_op, wn32_op)
{
	wire_id control(0, true);
	wire_id target(15, true);
	std::vector<gate> one_wire = {gate_lib::i, gate_lib::h,   gate_lib::x,
	                              gate_lib::y, gate_lib::z,   gate_lib::s,
	                              gate_lib::t, gate_lib::sdg, gate_lib::tdg};
	std::vector<gate> two_wire = {gate_lib::cx, gate_lib::cy, gate_lib::cz};

	for (gate const& g : one_wire) {
		check_one_wire<TestType>(g, target);
	}

	for (gate const& g : two_wire) {
		check_two_wire<TestType>(g, control, target);
	}

	TestType swap(gate_lib::swap, control, target);
	CHECK(swap.num_controls() == 0u);
	CHECK(swap.num_targets() == 2u);
	CHECK(swap.target(0) == control);
	CHECK(swap.target(1) == target);

	TestType swap_10(gate_lib::swap, target, control);
	CHECK(swap == swap_10);

	TestType swap_v(gate_lib::swap, std::vector<wire_id>({}),
	                std::vector<wire_id>({control, target}));
	CHECK(swap_v.num_controls() == 0u);
	CHECK(swap_v.num_targets() == 2u);
	CHECK(swap_v.target(0) == control);
	CHECK(swap_v.target(1) == target);

	TestType swap_v_10(gate_lib::swap, std::vector<wire_id>({}),
	                   std::vector<wire_id>({target, control}));
	CHECK(swap_v == swap_v_10);
}

TEMPLATE_TEST_CASE("Check correct instantiation of parameterasible 1 and 2 wires operations",
                   "[ops][inst]", w2_op, w3_op, wn32_op)
{
	wire_id control0(0, true);
	wire_id target(2, true);
	std::vector<angle> commmon_angles = {sym_angle::zero, sym_angle::pi, sym_angle::pi_half,
	                                     sym_angle::pi_quarter};
	for (angle const& a : commmon_angles) {
		check_one_wire<TestType>(gate_lib::r1(a), target);
		check_one_wire<TestType>(gate_lib::rx(a), target);
		check_one_wire<TestType>(gate_lib::ry(a), target);
		check_one_wire<TestType>(gate_lib::rz(a), target);
		check_two_wire<TestType>(gate_lib::crx(a), control0, target);
		check_two_wire<TestType>(gate_lib::cry(a), control0, target);
		check_two_wire<TestType>(gate_lib::crz(a), control0, target);
	}
}

template<class Op>
void check_three_wire(gate const& g, wire_id const& c0, wire_id const& c1, wire_id const& target)
{
	Op o(g, c0, c1, target);
	// Non-vector constructor
	CHECK(o.num_controls() == 2u);
	CHECK(o.num_targets() == 1u);
	CHECK(o.control(0) == c0);
	CHECK(o.control(1) == c1);
	CHECK(o.target() == target);

	Op o_norm(g, c1, c0, target);
	CHECK(o == o_norm);

	// Vector constructor
	Op o_vec(g, std::vector<wire_id>({c0, c1}), std::vector<wire_id>({target}));
	CHECK(o_vec.num_controls() == 2u);
	CHECK(o_vec.num_targets() == 1u);
	CHECK(o_vec.control(0) == c0);
	CHECK(o_vec.control(1) == c1);
	CHECK(o_vec.target() == target);

	CHECK(o == o_vec);

	Op o_vec_norm(g, std::vector<wire_id>({c1, c0}), std::vector<wire_id>({target}));
	CHECK(o_vec == o_vec_norm);
}

TEMPLATE_TEST_CASE("Check correct instantiation of non-parameterasible 3 wires operations",
                   "[ops][inst]", w3_op, wn32_op)
{
	wire_id control0(0, true);
	wire_id control1(8, true);
	wire_id target(15, true);
	std::vector<gate> three_wire = {gate_lib::ncx, gate_lib::ncy, gate_lib::ncz};

	for (gate const& g : three_wire) {
		check_three_wire<TestType>(g, control0, control1, target);
	}
}

TEMPLATE_TEST_CASE("Check correct instantiation of parameterasible 3 wires operations",
                   "[ops][inst]", w3_op, wn32_op)
{
	wire_id control0(0, true);
	wire_id control1(1, true);
	wire_id target(2, true);
	std::vector<angle> commmon_angles = {sym_angle::zero, sym_angle::pi, sym_angle::pi_half,
	                                     sym_angle::pi_quarter};
	for (angle const& a : commmon_angles) {
		check_three_wire<TestType>(gate_lib::ncrx(a), control0, control1, target);
		check_three_wire<TestType>(gate_lib::ncry(a), control0, control1, target);
		check_three_wire<TestType>(gate_lib::ncrz(a), control0, control1, target);
	}
}

TEMPLATE_TEST_CASE("Check 1 and 2 wires operations adjointness", "[ops][adj]", w2_op)
{
	wire_id q0(0, true);
	wire_id q1(9, true);
	wire_id q2(19, true);

	std::vector<gate> gs = {gate_lib::i, gate_lib::h, gate_lib::x,  gate_lib::y,  gate_lib::z,
	                        gate_lib::s, gate_lib::t, gate_lib::cx, gate_lib::cy, gate_lib::cz};
	std::vector<gate> gs_adjoint = {gate_lib::i,  gate_lib::h,   gate_lib::x,   gate_lib::y,
	                                gate_lib::z,  gate_lib::sdg, gate_lib::tdg, gate_lib::cx,
	                                gate_lib::cy, gate_lib::cz};
	std::vector<angle> commmon_angles = {sym_angle::zero, sym_angle::pi, sym_angle::pi_half,
	                                     sym_angle::pi_quarter};
	for (angle const& a : commmon_angles) {
		gs.emplace_back(gate_lib::r1(a));
		gs.emplace_back(gate_lib::rx(a));
		gs.emplace_back(gate_lib::ry(a));
		gs.emplace_back(gate_lib::rz(a));
		gs.emplace_back(gate_lib::crx(a));
		gs.emplace_back(gate_lib::cry(a));
		gs.emplace_back(gate_lib::crz(a));
		gs_adjoint.emplace_back(gate_lib::r1(-a));
		gs_adjoint.emplace_back(gate_lib::rx(-a));
		gs_adjoint.emplace_back(gate_lib::ry(-a));
		gs_adjoint.emplace_back(gate_lib::rz(-a));
		gs_adjoint.emplace_back(gate_lib::crx(-a));
		gs_adjoint.emplace_back(gate_lib::cry(-a));
		gs_adjoint.emplace_back(gate_lib::crz(-a));
	}

	std::vector<TestType> ops;
	std::vector<TestType> ops_adjoint;
	std::vector<TestType> ops_not_adjoint;
	for (uint32_t i = 0; i < gs.size(); ++i) {
		if (gs.at(i).is_one_qubit()) {
			ops.emplace_back(gs.at(i), q0);
			ops_adjoint.emplace_back(gs_adjoint.at(i), q0);
			ops_not_adjoint.emplace_back(gs.at(i), q1);
			ops_not_adjoint.emplace_back(gs_adjoint.at(i), q1);
			continue;
		} 
		ops.emplace_back(gs.at(i), q0, q1);
		ops_adjoint.emplace_back(gs_adjoint.at(i), q0, q1);
		ops_not_adjoint.emplace_back(gs.at(i), q0, q2);
		ops_not_adjoint.emplace_back(gs.at(i), q1, q0);
		ops_not_adjoint.emplace_back(gs_adjoint.at(i), q1, q0);
	}
	for (uint32_t i = 0; i < ops.size(); ++i) {
		CHECK(ops.at(i).is_adjoint(ops_adjoint.at(i)));
		CHECK(ops_adjoint.at(i).is_adjoint(ops.at(i)));
		for (uint32_t j = 0; j < ops.size(); ++j) {
			if (i == j) {
				continue;
			}
			CHECK_FALSE(ops.at(i).is_adjoint(ops.at(j)));
			CHECK_FALSE(ops.at(j).is_adjoint(ops.at(i)));
		}
		for (uint32_t j = 0; j < ops_not_adjoint.size(); ++j) {
			CHECK_FALSE(ops.at(i).is_adjoint(ops_not_adjoint.at(j)));
			CHECK_FALSE(ops_not_adjoint.at(j).is_adjoint(ops.at(i)));
		}
	}
	TestType swap_0(gate_lib::swap, q0, q1);
	TestType swap_1(gate_lib::swap, q1, q0);
	TestType swap_2(gate_lib::swap, q0, q2);
	TestType swap_3(gate_lib::swap, q1, q2);
	CHECK(swap_0.is_adjoint(swap_1));
	CHECK(swap_1.is_adjoint(swap_0));
	CHECK_FALSE(swap_0.is_adjoint(swap_2));
	CHECK_FALSE(swap_0.is_adjoint(swap_3));
	CHECK_FALSE(swap_2.is_adjoint(swap_3));
}

TEMPLATE_TEST_CASE("Check 1, 2 and 3 wires operations adjointness", "[ops][adj]", w3_op, wn32_op)
{
	wire_id q0(0, true);
	wire_id q1(9, true);
	wire_id q2(19, true);

	std::vector<gate> gs = {gate_lib::i,  gate_lib::h,  gate_lib::x,   gate_lib::y,
	                        gate_lib::z,  gate_lib::s,  gate_lib::t,   gate_lib::cx,
	                        gate_lib::cy, gate_lib::cz, gate_lib::ncx, gate_lib::ncy,
	                        gate_lib::ncz};
	std::vector<gate> gs_adjoint = {gate_lib::i,  gate_lib::h,   gate_lib::x,   gate_lib::y,
	                                gate_lib::z,  gate_lib::sdg, gate_lib::tdg, gate_lib::cx,
	                                gate_lib::cy, gate_lib::cz,  gate_lib::ncx, gate_lib::ncy,
	                                gate_lib::ncz};
	std::vector<angle> commmon_angles = {sym_angle::zero, sym_angle::pi, sym_angle::pi_half,
	                                     sym_angle::pi_quarter};
	for (angle const& a : commmon_angles) {
		gs.emplace_back(gate_lib::r1(a));
		gs.emplace_back(gate_lib::rx(a));
		gs.emplace_back(gate_lib::ry(a));
		gs.emplace_back(gate_lib::rz(a));
		gs.emplace_back(gate_lib::crx(a));
		gs.emplace_back(gate_lib::cry(a));
		gs.emplace_back(gate_lib::crz(a));
		gs.emplace_back(gate_lib::ncrx(a));
		gs.emplace_back(gate_lib::ncry(a));
		gs.emplace_back(gate_lib::ncrz(a));
		gs_adjoint.emplace_back(gate_lib::r1(-a));
		gs_adjoint.emplace_back(gate_lib::rx(-a));
		gs_adjoint.emplace_back(gate_lib::ry(-a));
		gs_adjoint.emplace_back(gate_lib::rz(-a));
		gs_adjoint.emplace_back(gate_lib::crx(-a));
		gs_adjoint.emplace_back(gate_lib::cry(-a));
		gs_adjoint.emplace_back(gate_lib::crz(-a));
		gs_adjoint.emplace_back(gate_lib::ncrx(-a));
		gs_adjoint.emplace_back(gate_lib::ncry(-a));
		gs_adjoint.emplace_back(gate_lib::ncrz(-a));
	}

	std::vector<TestType> ops;
	std::vector<TestType> ops_adjoint;
	std::vector<TestType> ops_not_adjoint;
	for (uint32_t i = 0; i < gs.size(); ++i) {
		if (gs.at(i).is_one_qubit()) {
			ops.emplace_back(gs.at(i), q0);
			ops_adjoint.emplace_back(gs_adjoint.at(i), q0);
			ops_not_adjoint.emplace_back(gs.at(i), q1);
			ops_not_adjoint.emplace_back(gs_adjoint.at(i), q1);
			continue;
		} else if (gs.at(i).is_two_qubit()) {
			ops.emplace_back(gs.at(i), q0, q1);
			ops_adjoint.emplace_back(gs_adjoint.at(i), q0, q1);
			ops_not_adjoint.emplace_back(gs.at(i), q0, q2);
			ops_not_adjoint.emplace_back(gs.at(i), q1, q0);
			ops_not_adjoint.emplace_back(gs_adjoint.at(i), q1, q0);
			continue;
		}
		ops.emplace_back(gs.at(i), q0, q1, q2);
		ops_adjoint.emplace_back(gs_adjoint.at(i), q0, q1, q2);
		ops_not_adjoint.emplace_back(gs.at(i), q0, q2, q1);
		ops_not_adjoint.emplace_back(gs.at(i), q1, q2, q0);
		ops_not_adjoint.emplace_back(gs_adjoint.at(i), q0, q2, q1);
	}
	for (uint32_t i = 0; i < ops.size(); ++i) {
		CHECK(ops.at(i).is_adjoint(ops_adjoint.at(i)));
		CHECK(ops_adjoint.at(i).is_adjoint(ops.at(i)));
		for (uint32_t j = 0; j < ops.size(); ++j) {
			if (i == j) {
				continue;
			}
			CHECK_FALSE(ops.at(i).is_adjoint(ops.at(j)));
			CHECK_FALSE(ops.at(j).is_adjoint(ops.at(i)));
		}
		for (uint32_t j = 0; j < ops_not_adjoint.size(); ++j) {
			CHECK_FALSE(ops.at(i).is_adjoint(ops_not_adjoint.at(j)));
			CHECK_FALSE(ops_not_adjoint.at(j).is_adjoint(ops.at(i)));
		}
	}
	TestType swap_0(gate_lib::swap, q0, q1);
	TestType swap_1(gate_lib::swap, q1, q0);
	TestType swap_2(gate_lib::swap, q0, q2);
	TestType swap_3(gate_lib::swap, q1, q2);
	CHECK(swap_0.is_adjoint(swap_1));
	CHECK(swap_1.is_adjoint(swap_0));
	CHECK_FALSE(swap_0.is_adjoint(swap_2));
	CHECK_FALSE(swap_0.is_adjoint(swap_3));
	CHECK_FALSE(swap_2.is_adjoint(swap_3));
}

template<class Op>
Op create_op(gate const& g, wire_id const& c0, wire_id const& c1, wire_id const& target)
{
	if (g.is_one_qubit()) {
		return Op(g, target);
	} else if (g.is_two_qubit()) {
		return Op(g, c0, target);
	}
	return Op(g, c0, c1, target);
}

TEMPLATE_TEST_CASE("Check 1 and 2 wires operations dependency", "[ops][dep]", w2_op)
{
	wire_id q0(0, true);
	wire_id q1(11, true);
	wire_id q2(12, true);
	wire_id q3(23, true);
	wire_id q4(24, true);
	wire_id q5(25, true);
	std::vector<gate> gs = {gate_lib::h,  gate_lib::x,  gate_lib::y,   gate_lib::z,
	                        gate_lib::s,  gate_lib::t,  gate_lib::sdg, gate_lib::tdg,
	                        gate_lib::cx, gate_lib::cy, gate_lib::cz};

	// All acting on different qubits
	for (uint32_t i = 0; i < gs.size(); ++i) {
		TestType op_i = create_op<TestType>(gs.at(i), q0, q1, q2);
		for (uint32_t j = i; j < gs.size(); ++j) {
			TestType op_j = create_op<TestType>(gs.at(j), q3, q4, q5);
			CHECK_FALSE((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
		}
	}

	// Here [q2, q3] are always targets, and [q0, q1] always controls. They don't cover any of
	// the more tricky dependency cases:
	for (uint32_t i = 0; i < gs.size(); ++i) {
		TestType op_i = create_op<TestType>(gs.at(i), q0, q1, q2);
		for (uint32_t j = i; j < gs.size(); ++j) {
			TestType op_j = create_op<TestType>(gs.at(j), q0, q1, q2);
			// Different target
			TestType op_dt = create_op<TestType>(gs.at(j), q0, q1, q3);
			CHECK_FALSE((op_i.is_dependent(op_dt) || op_dt.is_dependent(op_i)));
			if (i == j) {
				CHECK_FALSE((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
				continue;
			}
			if (gs.at(i).axis() == gs.at(j).axis()) {
				CHECK_FALSE((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
				continue;
			}
			CHECK((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
		}
	}

	for (uint32_t i = 0; i < gs.size(); ++i) {
		TestType op_i = create_op<TestType>(gs.at(i), q0, q1, q2);
		for (uint32_t j = 8; j < gs.size(); ++j) {
			TestType op_j = create_op<TestType>(gs.at(j), q2, q1, q0);
			if (gs.at(i).is_one_qubit() && gs.at(i).axis() == rot_axis::z) {
				CHECK_FALSE((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
				continue;
			} else if (gs.at(i).axis() == rot_axis::z && gs.at(j).axis() == rot_axis::z) {
				CHECK_FALSE((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
				continue;
			}
			CHECK((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
		}
	}

	// Two-qubit gates: same target, different controls
	for (uint32_t i = 8; i < gs.size(); ++i) {
		TestType op_i = create_op<TestType>(gs.at(i), q0, q1, q2);
		for (uint32_t j = i; j < gs.size(); ++j) {
			TestType op_j = create_op<TestType>(gs.at(j), q3, q4, q2);
			if (gs.at(i).axis() == gs.at(j).axis()) {
				CHECK_FALSE((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
				continue;
			}
			CHECK((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
		}
	}

	// The swap
	TestType swap_op0(gate_lib::swap, q0, q2);
	TestType swap_op1(gate_lib::swap, q1, q3);
	TestType swap_op3(gate_lib::swap, q4, q5);
	CHECK_FALSE((swap_op0.is_dependent(swap_op1) || swap_op1.is_dependent(swap_op0)));
	for (uint32_t i = 0; i < gs.size(); ++i) {
		TestType op_i0 = create_op<TestType>(gs.at(i), q0, q1, q2);
		CHECK((op_i0.is_dependent(swap_op0) || swap_op0.is_dependent(op_i0)));
		if (gs.at(i).is_one_qubit() || gs.at(i).is_two_qubit()) {
			CHECK_FALSE((op_i0.is_dependent(swap_op1) || swap_op1.is_dependent(op_i0)));
		} else {
			CHECK((op_i0.is_dependent(swap_op1) || swap_op1.is_dependent(op_i0)));
		}
		CHECK_FALSE((op_i0.is_dependent(swap_op3) || swap_op3.is_dependent(op_i0)));
	}
}

TEMPLATE_TEST_CASE("Check 1, 2 and 3 wires dependency", "[ops][dep]", w3_op, wn32_op)
{
	wire_id q0(0, true);
	wire_id q1(11, true);
	wire_id q2(12, true);
	wire_id q3(23, true);
	wire_id q4(24, true);
	wire_id q5(25, true);
	std::vector<gate> gs = {gate_lib::h,   gate_lib::x,  gate_lib::y,   gate_lib::z,
	                        gate_lib::s,   gate_lib::t,  gate_lib::sdg, gate_lib::tdg,
	                        gate_lib::cx,  gate_lib::cy, gate_lib::cz,  gate_lib::ncx,
	                        gate_lib::ncy, gate_lib::ncz};

	// All acting on different qubits
	for (uint32_t i = 0; i < gs.size(); ++i) {
		TestType op_i = create_op<TestType>(gs.at(i), q0, q1, q2);
		for (uint32_t j = i; j < gs.size(); ++j) {
			TestType op_j = create_op<TestType>(gs.at(j), q3, q4, q5);
			CHECK_FALSE((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
		}
	}

	// Here [q2, q3] are always targets, and [q0, q1] always controls. They don't cover any of
	// the more tricky dependency cases:
	for (uint32_t i = 0; i < gs.size(); ++i) {
		TestType op_i = create_op<TestType>(gs.at(i), q0, q1, q2);
		for (uint32_t j = i; j < gs.size(); ++j) {
			TestType op_j = create_op<TestType>(gs.at(j), q0, q1, q2);
			// Different target
			TestType op_dt = create_op<TestType>(gs.at(j), q0, q1, q3);
			CHECK_FALSE((op_i.is_dependent(op_dt) || op_dt.is_dependent(op_i)));
			if (i == j) {
				CHECK_FALSE((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
				continue;
			}
			if (gs.at(i).axis() == gs.at(j).axis()) {
				CHECK_FALSE((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
				continue;
			}
			CHECK((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
		}
	}

	for (uint32_t i = 0; i < gs.size(); ++i) {
		TestType op_i = create_op<TestType>(gs.at(i), q0, q1, q2);
		for (uint32_t j = 8; j < gs.size(); ++j) {
			TestType op_j = create_op<TestType>(gs.at(j), q2, q1, q0);
			if (gs.at(i).is_one_qubit() && gs.at(i).axis() == rot_axis::z) {
				CHECK_FALSE((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
				continue;
			} else if (gs.at(i).axis() == rot_axis::z && gs.at(j).axis() == rot_axis::z) {
				CHECK_FALSE((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
				continue;
			}
			CHECK((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
		}
	}

	// Two-qubit and N-qubit gates: same target, different controls
	for (uint32_t i = 8; i < gs.size(); ++i) {
		TestType op_i = create_op<TestType>(gs.at(i), q0, q1, q2);
		for (uint32_t j = i; j < gs.size(); ++j) {
			TestType op_j = create_op<TestType>(gs.at(j), q3, q4, q2);
			if (gs.at(i).axis() == gs.at(j).axis()) {
				CHECK_FALSE((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
				continue;
			}
			CHECK((op_i.is_dependent(op_j) || op_j.is_dependent(op_i)));
		}
	}

	// Some edge cases:
	TestType cz_op(gate_lib::cz, q0, q1);
	TestType ccx_op(gate_lib::ncx, q0, q1, q2);
	TestType ccy_op(gate_lib::ncy, q0, q1, q2);
	TestType ccz_op(gate_lib::ncz, q0, q1, q2);
	CHECK_FALSE((cz_op.is_dependent(ccx_op) || ccx_op.is_dependent(cz_op)));
	CHECK_FALSE((cz_op.is_dependent(ccy_op) || ccy_op.is_dependent(cz_op)));
	CHECK_FALSE((cz_op.is_dependent(ccz_op) || ccz_op.is_dependent(cz_op)));

	// The swap
	TestType swap_op0(gate_lib::swap, q0, q2);
	TestType swap_op1(gate_lib::swap, q1, q3);
	TestType swap_op3(gate_lib::swap, q4, q5);
	CHECK_FALSE((swap_op0.is_dependent(swap_op1) || swap_op1.is_dependent(swap_op0)));
	for (uint32_t i = 0; i < gs.size(); ++i) {
		TestType op_i0 = create_op<TestType>(gs.at(i), q0, q1, q2);
		CHECK((op_i0.is_dependent(swap_op0) || swap_op0.is_dependent(op_i0)));
		if (gs.at(i).is_one_qubit() || gs.at(i).is_two_qubit()) {
			CHECK_FALSE((op_i0.is_dependent(swap_op1) || swap_op1.is_dependent(op_i0)));
		} else {
			CHECK((op_i0.is_dependent(swap_op1) || swap_op1.is_dependent(op_i0)));
		}
		CHECK_FALSE((op_i0.is_dependent(swap_op3) || swap_op3.is_dependent(op_i0)));
	}
}