/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Mapping/sabre_map.h"

#include "../check_mapping.h"
#include "test_circuits.h"
#include "tweedledum/IR/Circuit.h"
#include "tweedledum/Passes/Optimization/steiner_resynth.h"
#include "tweedledum/Parser/qasm.h"
#include "tweedledum/Target/Device.h"

#include <catch.hpp>
#include <filesystem>
#include <string>

TEST_CASE("sabre_map test cases", "[sabre_map][mapping]")
{
    using namespace tweedledum;
    SECTION("Test circuit 00")
    {
        Circuit original = test_circuit_00();
        Device device = Device::path(original.num_qubits());
        auto [mapped, mapping] = sabre_map(device, original);
        CHECK(check_mapping(device, original, mapped, mapping));
    }
    SECTION("Test circuit 01")
    {
        Circuit original = test_circuit_01();
        Device device = Device::path(original.num_qubits());
        auto [mapped, mapping] = sabre_map(device, original);
        CHECK(check_mapping(device, original, mapped, mapping));
    }
    SECTION("Test circuit 02")
    {
        Circuit original = test_circuit_02();
        Device device = Device::path(original.num_qubits());
        auto [mapped, mapping] = sabre_map(device, original);
        CHECK(check_mapping(device, original, mapped, mapping));
    }
    SECTION("Test circuit 03")
    {
        Circuit original = test_circuit_03();
        Device device = Device::path(original.num_qubits());
        auto [mapped, mapping] = sabre_map(device, original);
        CHECK(check_mapping(device, original, mapped, mapping));
    }
    SECTION("Test circuit 04")
    {
        Circuit original = test_circuit_04();
        Device device = Device::path(original.num_qubits());
        auto [mapped, mapping] = sabre_map(device, original);
        CHECK(check_mapping(device, original, mapped, mapping));
    }
    SECTION("Test circuit 05")
    {
        Circuit original = test_circuit_05();
        Device device = Device::path(original.num_qubits());
        auto [mapped, mapping] = sabre_map(device, original);
        CHECK(check_mapping(device, original, mapped, mapping));
    }
    SECTION("Test circuit 06")
    {
        Circuit original = test_circuit_06();
        Device device = Device::path(original.num_qubits());
        auto [mapped, mapping] = sabre_map(device, original);
        CHECK(check_mapping(device, original, mapped, mapping));
    }
    SECTION("Test circuit 07")
    {
        Circuit original = test_circuit_07();
        Device device = Device::path(original.num_qubits());
        auto [mapped, mapping] = sabre_map(device, original);
        CHECK(check_mapping(device, original, mapped, mapping));
    }
}

// TODO: Fix it on windows?
#if defined(TEST_QASM_DIR) && !defined(_WIN32)
TEST_CASE("QASM circuits, mapping", "[sabre_map][mapping]")
{
    #define QASM_DIR TEST_QASM_DIR
    namespace fs = std::filesystem;
    fs::path qasm_dir = QASM_DIR;
    Device device = Device::from_edge_list({{0, 1}, {1, 2}, {2, 3}, {3, 4},
      {0, 5}, {1, 6}, {1, 7}, {2, 6}, {2, 7}, {3, 8}, {3, 9}, {4, 8}, {4, 9},
      {5, 6}, {6, 7}, {7, 8}, {8, 9}, {5, 10}, {5, 11}, {6, 10}, {6, 11},
      {7, 12}, {7, 13}, {8, 12}, {8, 13}, {9, 14}, {10, 11}, {11, 12}, {12, 13},
      {13, 14}, {10, 15}, {11, 16}, {11, 17}, {12, 16}, {12, 17}, {13, 18},
      {13, 19}, {14, 18}, {14, 19}, {15, 16}, {16, 17}, {17, 18}, {18, 19}});
    for (auto const& entry : fs::directory_iterator(qasm_dir)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        std::string filepath(entry.path().c_str());
        Circuit original = qasm::parse_source_file(filepath);
        for (uint32_t i = original.num_qubits(); i < device.num_qubits(); ++i) {
            original.create_qubit();
        }
        auto [mapped, mapping] = sabre_map(device, original);
        INFO("File: " << entry.path().filename());
        CHECK(check_mapping(device, original, mapped, mapping));
    }
}

TEST_CASE("QASM circuits, mapping + resynthesis", "[sabre_map][mapping]")
{
    #define QASM_DIR TEST_QASM_DIR
    namespace fs = std::filesystem;
    fs::path qasm_dir = QASM_DIR;
    Device device = Device::from_edge_list({{0, 1}, {1, 2}, {2, 3}, {3, 4},
      {0, 5}, {1, 6}, {1, 7}, {2, 6}, {2, 7}, {3, 8}, {3, 9}, {4, 8}, {4, 9},
      {5, 6}, {6, 7}, {7, 8}, {8, 9}, {5, 10}, {5, 11}, {6, 10}, {6, 11},
      {7, 12}, {7, 13}, {8, 12}, {8, 13}, {9, 14}, {10, 11}, {11, 12}, {12, 13},
      {13, 14}, {10, 15}, {11, 16}, {11, 17}, {12, 16}, {12, 17}, {13, 18},
      {13, 19}, {14, 18}, {14, 19}, {15, 16}, {16, 17}, {17, 18}, {18, 19}});
    for (auto const& entry : fs::directory_iterator(qasm_dir)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        std::string filepath(entry.path().c_str());
        Circuit original = qasm::parse_source_file(filepath);
        for (uint32_t i = original.num_qubits(); i < device.num_qubits(); ++i) {
            original.create_qubit();
        }
        auto [mapped, mapping] = sabre_map(device, original);
        Circuit optimized = steiner_resynth(device, mapped);
        INFO("File: " << entry.path().filename());
        CHECK(check_mapping(device, original, optimized, mapping));
    }
}
#endif
