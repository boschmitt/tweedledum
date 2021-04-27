/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Mapping/sabre_map.h"

#include "../check_mapping.h"
#include "test_circuits.h"
#include "tweedledum/IR/Circuit.h"
#include "tweedledum/Target/Device.h"

#include <catch.hpp>

TEST_CASE("sabre_map test cases", "[sabre_map][mapping]")
{
    using namespace tweedledum;
    SECTION("Test circuit 00") {
        Circuit original = test_circuit_00();
        Device device = Device::path(original.num_qubits());
        auto [mapped, mapping] = sabre_map(device, original);
        CHECK(check_mapping(device, original, mapped, mapping));
    }
    SECTION("Test circuit 01") {
        Circuit original = test_circuit_01();
        Device device = Device::path(original.num_qubits());
        auto [mapped, mapping] = sabre_map(device, original);
        CHECK(check_mapping(device, original, mapped, mapping));
    }
    SECTION("Test circuit 02") {
        Circuit original = test_circuit_02();
        Device device = Device::path(original.num_qubits());
        auto [mapped, mapping] = sabre_map(device, original);
        CHECK(check_mapping(device, original, mapped, mapping));
    }
    SECTION("Test circuit 03") {
        Circuit original = test_circuit_03();
        Device device = Device::path(original.num_qubits());
        auto [mapped, mapping] = sabre_map(device, original);
        CHECK(check_mapping(device, original, mapped, mapping));
    }
    SECTION("Test circuit 04") {
        Circuit original = test_circuit_04();
        Device device = Device::path(original.num_qubits());
        auto [mapped, mapping] = sabre_map(device, original);
        CHECK(check_mapping(device, original, mapped, mapping));
    }
    SECTION("Test circuit 05") {
        Circuit original = test_circuit_05();
        Device device = Device::path(original.num_qubits());
        auto [mapped, mapping] = sabre_map(device, original);
        CHECK(check_mapping(device, original, mapped, mapping));
    }
    SECTION("Test circuit 06") {
        Circuit original = test_circuit_06();
        Device device = Device::path(original.num_qubits());
        auto [mapped, mapping] = sabre_map(device, original);
        CHECK(check_mapping(device, original, mapped, mapping));
    }
    SECTION("Test circuit 07") {
        Circuit original = test_circuit_07();
        Device device = Device::path(original.num_qubits());
        auto [mapped, mapping] = sabre_map(device, original);
        CHECK(check_mapping(device, original, mapped, mapping));
    }
}
