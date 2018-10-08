/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

/* Taken from mockturtle (: */

#include "../traits.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace tweedledum {

/*! \brief Associative container for network nodes
 *
 * This container helps to store values associated to nodes in a network.  The
 * container is initialized with a network to derive the size according to the
 * number of nodes. The container can be accessed via nodes, or indirectly
 * via node_ptr, from which the corresponding node is derived.
 *
 * The implementation uses a vector as underlying data structure which is
 * indexed by the node's index.
 *
 * **Required network functions:**
 * - `size`
 * - `get_node`
 * - `node_to_index`
 */
template<class T, class Network>
class node_map {
public:
	using reference = typename std::vector<T>::reference;
	using const_reference = typename std::vector<T>::const_reference;

public:
	/*! \brief Default constructor. */
	explicit node_map(Network const& ntk)
	    : ntk(ntk)
	    , data(std::make_shared<std::vector<T>>(ntk.size()))
	{}

	/*! \brief Constructor with default value.
	 *
	 * Initializes all values in the container to `init_value`.
	 */
	node_map(Network const& ntk, T const& init_value)
	    : ntk(ntk)
	    , data(std::make_shared<std::vector<T>>(ntk.size(), init_value))
	{}

	/*! \brief Mutable access to value by node. */
	reference operator[](node<Network> const& n)
	{
		assert(ntk.node_to_index(n) < data->size() && "index out of bounds");
		return (*data)[ntk.node_to_index(n)];
	}

	/*! \brief Constant access to value by node. */
	const_reference operator[](node<Network> const& n) const
	{
		assert(ntk.node_to_index(n) < data->size() && "index out of bounds");
		return (*data)[ntk.node_to_index(n)];
	}

	/*! \brief Mutable access to value by node_ptr.
	 *
	 * This method derives the node from the node_ptr.  If the node and node_ptr type
	 * are the same in the network implementation, this method is disabled.
	 */
	template<typename _Ntk = Network,
	         typename = std::enable_if_t<!std::is_same_v<node_ptr<_Ntk>, node<_Ntk>>>>
	reference operator[](node_ptr<Network> const& f)
	{
		assert(ntk.node_to_index(ntk.get_node(f)) < data->size() && "index out of bounds");
		return (*data)[ntk.node_to_index(ntk.get_node(f))];
	}

	/*! \brief Constant access to value by node_ptr.
	 *
	 * This method derives the node from the node_ptr.  If the node and node_ptr type
	 * are the same in the network implementation, this method is disabled.
	 */
	template<typename _Ntk = Network,
	         typename = std::enable_if_t<!std::is_same_v<node_ptr<_Ntk>, node<_Ntk>>>>
	const_reference operator[](node_ptr<Network> const& f) const
	{
		assert(ntk.node_to_index(ntk.get_node(f)) < data->size() && "index out of bounds");
		return (*data)[ntk.node_to_index(ntk.get_node(f))];
	}

	/*! \brief Resets the size of the map.
	 *
	 * This function should be called, if the network changed in size. Then,
	 * the map is cleared, and resized to the current network's size.  All
	 * values are initialized with `init_value`.
	 *
	 * \param init_value Initialization value after resize
	 */
	void reset(T const& init_value = {})
	{
		data->clear();
		data->resize(ntk.size(), init_value);
	}

	/*! \brief Resizes the map.
	 *
	 * This function should be called, if the node_map's size needs to
	 * be changed without clearing its data.
	 *
	 * \param init_value Initialization value after resize
	 */
	void resize(T const& init_value = {})
	{
		if (ntk.size() > data->size()) {
			data->resize(ntk.size(), init_value);
		}
	}

private:
	Network const& ntk;
	std::shared_ptr<std::vector<T>> data;
};

} // namespace tweedledum