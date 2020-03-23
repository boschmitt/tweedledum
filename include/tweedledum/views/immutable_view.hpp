/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

namespace tweedledum {

/*! \brief Deletes all methods that can change the network.
 *
 * This view deletes all methods that can change the network structure such as
 * This view is convenient to use as a base class for other views that make some
 * computations based on the structure when being constructed.
 */
template<typename Network>
class immutable_view : public Network {
public:
	using gate_type = typename Network::gate_type;
	using node_type = typename Network::node_type;
	using storage_type = typename Network::storage_type;

	/*! \brief Default constructor.
	 *
	 * Constructs immutable view on a network.
	 */
	immutable_view(Network const& network)
	    : Network(network)
	{}
};

} // namespace tweedledum
