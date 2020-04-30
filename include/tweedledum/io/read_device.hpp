/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "../target/device.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <istream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <utility>

namespace tweedledum {
#pragma region utility functions(for readability)
namespace utility {

inline void left_trim(std::string& str, const char* chars = " \t\n\v\f\r")
{
	if (!str.empty()) {
		const auto str_idx = str.find_first_not_of(chars);

		if (str_idx == std::string::npos) {
			str.clear();
		} else if (str_idx > 0) {
			str = str.substr(str_idx, std::string::npos);
		}
	}
}

inline auto split(std::string const& str)
{
	std::vector<std::string> split_string;
	std::istringstream iss(str);
	std::copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(),
	          std::back_inserter(split_string));
	return split_string;
}

} // namespace utility
#pragma endregion

/*! \brief Parse a device coupling graph from an input stream
 *
 * This is the input stream variant of ``read_device``, in which an input stream is passed as a
 * parameter.
 * 
 * \param in Input stream (default: std::cin)
 * \returns A device structure
 */
inline device read_device(std::istream& in = std::cin)
{
	using namespace utility;
	std::string line;
	std::getline(in, line);

	auto const num_nodes = std::stoll(line);
	device arch(num_nodes);
	while (std::getline(in, line)) {
		left_trim(line);
		if (line.empty()) {
			continue;
		}
		auto const s = split(line);
		auto const v = std::stoi(s[0]);
		auto const w = std::stoi(s[1]);
		arch.add_edge(v, w);
	}
	return arch;
}

/*! \brief Parse a device coupling graph from a file
 *
 * The file format is rather simple: the first line has one unsigned integer n which gives the
 * number of nodes. Nodes are identified by numbers between 0 and (n - 1). All subsequent
 * lines are pairs of unsigned integers (nodes identifiers) representing undirected edges
 * between nodes.
 * 
 * \param filename Filename string
 * \returns A device
 */
inline device read_device(std::string_view filename)
{
	std::ifstream in(filename, std::ios::in);
	return read_device(in);
}

/*! \brief Parse device information from a JSON file
 *
 * \param filename JSON file name
 * \returns A device
 */
inline device read_device_from_json(std::string_view filename)
{
	using json = nlohmann::json;

	std::ifstream input(filename, std::ios::in);
	json device_info;
	input >> device_info;

	uint32_t num_qubits = device_info["n_qubits"];
	std::string name = device_info["backend_name"];
	device d(num_qubits, name);
	for (auto value : device_info["coupling_map"]) {
		uint32_t v = value[0];
		uint32_t w = value[1];
		d.add_edge(v, w);
	}
	return d;
}

} // namespace tweedledum