/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gate_kinds.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace tweedledee {

namespace utils {
inline void left_trim(std::string& str, const char* chars = " \t\n\v\f\r")
{
	if (!str.empty()) {
		std::size_t str_idx = str.find_first_not_of(chars);

		if (str_idx == std::string::npos)
			str.clear();
		else if (str_idx > 0)
			str = str.substr(str_idx, std::string::npos);
	}
}

inline std::vector<std::string> split(std::string const& str)
{
	std::vector<std::string> slipt_string;
	std::istringstream iss(str);
	std::copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(),
	          std::back_inserter(slipt_string));
	return slipt_string;
}
} // namespace utils

namespace detail {
struct identify_gate_kind {
	gate_kinds operator()(std::string_view gate_label)
	{
		switch (gate_label[0]) {
		case 'H':
			return gate_kinds::hadamard;

		case 'S':
		case 'P':
			if (gate_label.size() == 2 && gate_label[1] == '*') {
				return gate_kinds::phase_dagger;
			}
			return gate_kinds::phase;

		case 'T':
			if (gate_label.size() == 2 && gate_label[1] == '*') {
				return gate_kinds::t_dagger;
			}
			return gate_kinds::t;

		case 'X':
			return gate_kinds::pauli_x;

		case 'Y':
			return gate_kinds::pauli_y;

		case 'Z':
			return gate_kinds::pauli_z;

		default:
			break;
		}
		if (gate_label == "tof") {
			return gate_kinds::cnot;
		}
		return gate_kinds::unknown;
	}
};
} // namespace detail

template<typename GateKind = gate_kinds>
class dotqc_reader {
public:
	virtual void on_qubit(std::string label)
	{
		(void) label;
	}

	virtual void on_input(std::string label)
	{
		(void) label;
	}

	virtual void on_output(std::string label)
	{
		(void) label;
	}

	virtual void on_gate(GateKind kind, std::string const& target)
	{
		(void) kind;
		(void) target;
	}

	virtual void on_gate(GateKind kind, std::vector<std::string> const& controls,
	                     std::vector<std::string> const& targets)
	{
		(void) kind;
		(void) controls;
		(void) targets;
	}

	virtual void on_end()
	{}
};

template<typename GateKind, class Fn = detail::identify_gate_kind>
inline void dotqc_read(std::istream& buffer, dotqc_reader<GateKind>& reader, Fn&& fn = {})
{
	std::string line;

	while (buffer.peek() == '.' || buffer.peek() == '#') {
		if (buffer.peek() == '#') {
			std::getline(buffer, line);
			continue;
		}
		std::getline(buffer, line, ' ');
		if (line[1] == 'v') {
			std::getline(buffer, line);
			auto qubits = utils::split(line);
			for (auto& label : qubits) {
				reader.on_qubit(label);
			}
		} else if (line[1] == 'i') {
			std::getline(buffer, line);
			auto inputs = utils::split(line);
			for (auto& label : inputs) {
				reader.on_input(label);
			}
		} else if (line[1] == 'o') {
			std::getline(buffer, line);
			auto outputs = utils::split(line);
			for (auto& label : outputs) {
				reader.on_output(label);
			}
		} else {
			// Ignore unknown directive line.
			std::getline(buffer, line);
		}
	}

	while (std::getline(buffer, line)) {
		utils::left_trim(line);
		if (line.empty()) {
			continue;
		}
		auto entries = utils::split(line);
		if (entries[0] == "BEGIN" || entries[0] == "END") {
			continue;
		}
		auto gate = fn(entries[0]);
		entries.erase(entries.begin());
		switch (entries.size()) {
		case 0:
			/* Hopefully was either BEGIN or END */
			break;

		case 1:
			reader.on_gate(gate, entries[0]);
			break;

		default:
			reader.on_gate(gate,
			               std::vector<std::string>(entries.begin(), entries.end() - 1),
			               std::vector({entries.back()}));
			break;
		}
	}
}

template<typename GateKind, class Fn = detail::identify_gate_kind>
inline void dotqc_read(std::string const& path, dotqc_reader<GateKind>& reader, Fn&& fn = {})
{
	// Load the whole file into a buffer
	std::ifstream input_file(path);
	if (!input_file.is_open()) {
		std::cerr << "[e] Couldn't open file: " << path << '\n';
		return;
	}
	dotqc_read(input_file, reader, fn);
	input_file.close();
}

} // namespace tweedledee
