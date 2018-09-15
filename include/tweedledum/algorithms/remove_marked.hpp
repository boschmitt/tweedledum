/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

namespace tweedledum {

/*! \brief Removes marked nodes.
 *
 * This method reconstructs a network and omits all marked nodes. The
 * network types of the source and destination can be different.
 *
 * **Required network functions:**
 * - `add_gate`
 * - `foreach_qubit`
 * - `foreach_gate`
 * - `mark`
 */
template<typename NetworkSrc, typename NetworkDest>
void remove_marked(NetworkSrc const& src, NetworkDest& dest)
{
	src.foreach_gate([&](auto const& node) {
		if (src.mark(node)) {
			return;
		}
		dest.add_gate(node.gate);
	});
}

/*! \brief Removes marked nodes.
 *
 * This method reconstructs a network and omits all marked nodes. The
 * network types of the source and destination network are the same.
 *
 * **Required network functions:**
 * - `add_gate`
 * - `foreach_qubit`
 * - `foreach_gate`
 * - `mark`
 */
template<typename Network>
Network remove_marked(Network const& src)
{
	Network dest;
	src.foreach_qubit([&](auto id, auto& qubit_label) {
		(void) id;
		dest.add_qubit(qubit_label);
	});
	remove_marked(src, dest);
	return dest;
}

} // namespace tweedledum
