/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken, Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#pragma once

/* Taken from mockturtle (: */

#include "../traits.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace tweedledum {

/*! \brief Associative container for network vertices
 *
 * This container helps to store values associated to vertices in a network.  The
 * container is initialized with a network to derive the size according to the
 * number of vertices. The container can be accessed via vertices, or indirectly
 * via `link_type`, from which the corresponding vertex is derived.
 *
 * The implementation uses a vector as underlying data_ structure which is
 * indexed by the vertex's index.
 *
 */
template<class T, class Network>
class vertex_map {
public:
	using vertex_type = typename Network::vertex_type;
	using link_type = typename Network::link_type;
	using reference = typename std::vector<T>::reference;
	using const_reference = typename std::vector<T>::const_reference;

public:
	/*! \brief Default constructor. */
	explicit vertex_map(Network const& network)
	    : network_(network)
	    , data_(std::make_shared<std::vector<T>>(network.size()))
	{}

	/*! \brief Constructor with default value.
	 *
	 * Initializes all values in the container to `init_value`.
	 */
	vertex_map(Network const& network, T const& init_value)
	    : network_(network)
	    , data_(std::make_shared<std::vector<T>>(network.size(), init_value))
	{}

	/*! \brief Mutable access to value by vertex. */
	reference operator[](vertex_type const& vertex)
	{
		assert(network_.index(vertex) < data_->size() && "index out of bounds");
		return (*data_)[network_.index(vertex)];
	}

	/*! \brief Constant access to value by vertex. */
	const_reference operator[](vertex_type const& vertex) const
	{
		assert(network_.index(vertex) < data_->size() && "index out of bounds");
		return (*data_)[network_.index(vertex)];
	}

	/*! \brief Mutable access to value by `link_type`.
	 *
	 * This method derives the vertex from the `link_type`.  If the vertex and `link_type` type
	 * are the same in the network implementation, this method is disabled.
	 */
	template<typename _Ntk = Network,
	         typename = std::enable_if_t<!std::is_same_v<typename _Ntk::link_type,
		                                             typename _Ntk::vertex_type>>>
	reference operator[](link_type const& f)
	{
		assert(network_.node_to_index(network_.get_node(f)) < data_->size()
		       && "index out of bounds");
		return (*data_)[network_.node_to_index(network_.get_node(f))];
	}

	/*! \brief Constant access to value by `link_type`.
	 *
	 * This method derives the vertex from the `link_type`.  If the vertex and `link_type` type
	 * are the same in the network implementation, this method is disabled.
	 */
	template<typename _Ntk = Network,
	         typename = std::enable_if_t<!std::is_same_v<typename _Ntk::link_type,
		                                             typename _Ntk::vertex_type>>>
	const_reference operator[](link_type const& f) const
	{
		assert(network_.node_to_index(network_.get_node(f)) < data_->size()
		       && "index out of bounds");
		return (*data_)[network_.node_to_index(network_.get_node(f))];
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
		data_->clear();
		data_->resize(network_.size(), init_value);
	}

	/*! \brief Resizes the map.
	 *
	 * This function should be called, if the vertex_map's size needs to
	 * be changed without clearing its data.
	 *
	 * \param init_value Initialization value after resize
	 */
	void resize(T const& init_value = {})
	{
		if (network_.size() > data_->size()) {
			data_->resize(network_.size(), init_value);
		}
	}

private:
	Network const& network_;
	std::shared_ptr<std::vector<T>> data_;
};

} // namespace tweedledum